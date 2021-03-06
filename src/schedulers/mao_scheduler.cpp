#include "mao_scheduler.h"

#include "tasks_graph.h"

#include <numeric>

using std::map;
using std::max;
using std::pair;
using std::queue;
using std::set;
using std::shared_ptr;
using std::unique_ptr;
using std::string;
using std::vector;

XBT_LOG_NEW_DEFAULT_CATEGORY(mao_scheduler, "Mao scheduler log");

MaoScheduler::Actions MaoScheduler::PrepareForRun(View::Viewer& v) {
    viewer.reset(&v);

    vector<VMDescription> taskVM = GetCheapestVMs();

    // TasksBundling(taskVM);
    
    vector<View::Task> taskOrder = v.MakeTasksOrder();

    while (true) {
        double makespan = CalculateMakespan(taskVM, taskOrder);

        if (makespan <= v.GetDeadline()) {
            break;
        }

        XBT_INFO("Old makespan: %f", makespan);
        ReduceMakespan(taskVM, taskOrder, makespan);
        XBT_INFO("New makespan: %f", CalculateMakespan(taskVM, taskOrder));
    }

    vector<pair<double, double>> deadlines = CalculateDeadlines(v, taskOrder, taskVM);

    vector<vector<LoadVectorEvent>> loadVector = GetLoadVector(deadlines, taskVM);

    vector<LoadVectorEvent> sortedEvents;
    for (const auto& vmLoadVector: loadVector) {
        sortedEvents.insert(sortedEvents.end(), vmLoadVector.begin(), vmLoadVector.end());
    }
    std::sort(sortedEvents.begin(), sortedEvents.end(), [](const auto& a, const auto& b) {
        return a.End < b.End;
    });

    MaoScheduler::Actions actions;

    Schedule s;

    int vmId = 0;
    for (const VMDescription& vmDescr: taskVM) {
        actions.push_back(std::make_shared<BuyVMAction>(vmDescr, vmId));
        // FIXME if item.Id is not equal to task position in taskList
        s.AddItem(vmId, ScheduleItem(vmId));
        ++vmId;
    }

    actions.push_back(std::make_shared<ResetScheduleAction>(s));

    XBT_INFO("Schedule created!");
    XBT_INFO("Makespan: %f", CalculateMakespan(taskVM, taskOrder));
    XBT_INFO("Cost: %f", CalculateCost(taskVM));

    return actions;
}

vector<VMDescription> MaoScheduler::GetCheapestVMs() {
    vector<VMDescription> cheapestVM;
    
    for (auto taskIterator = viewer->GetGraphIterator(); *taskIterator; taskIterator->Next()) {
        auto task = taskIterator->Get();
        int bestPrice = -1;
        VMDescription bestVM;
        for (VMDescription vmDescr: viewer->GetVMList()) {
            // we probably want to reconsider way of choosing most efficient vm type for a task
            if (task.CanBeExecuted(vmDescr) &&
                (bestPrice == -1 || vmDescr.GetPrice() < bestPrice)) {
                bestPrice = vmDescr.GetPrice();
                bestVM = vmDescr;
            }
        }
        xbt_assert(bestPrice != -1, "No suitable VM for task %d found!", task.GetId());
        cheapestVM.push_back(bestVM);
    }
    return cheapestVM;
}

double MaoScheduler::CalculateCost(const vector<VMDescription>& taskVM) const {
    double cost = 0;
    for (size_t taskId = 0; taskId < viewer->WorkflowSize(); ++taskId) {
        VMDescription vmDescr = taskVM[taskId];
        const View::Task& task = viewer->GetTaskById(taskId);
        // FIXME when startup cost is added
        cost += task.GetExecutionCost(vmDescr);
    }
    return cost;
}

// we should enhance this in future so that data transer time is considered
double MaoScheduler::CalculateMakespan(const vector<VMDescription>& taskVM, const vector<View::Task>& taskOrder) {
    double makespan = 0;

    vector<double> endTime(taskOrder.size(), -1);
    double avgLag = static_cast<double>(viewer->GetVMList().GetMinLag() + viewer->GetVMList().GetMaxLag()) / 2;

    for (const View::Task& task: taskOrder) {
        double earliestBegin = 0;
        for (int dependencyId: task.GetDependencies()) {
            xbt_assert(endTime[dependencyId] != -1, "Something went wrong, task order is inconsistent!");
            earliestBegin = max(earliestBegin, endTime[dependencyId]);
        }
        earliestBegin += avgLag;
        endTime[task.GetId()] = earliestBegin + task.GetExecutionTime(taskVM[task.GetId()]);
        makespan = max(makespan, endTime[task.GetId()]);
    }

    return makespan;
}

void MaoScheduler::ReduceMakespan(vector<VMDescription>& taskVM, 
                                  const vector<View::Task>& taskOrder, 
                                  double currentMakespan) {
    XBT_INFO("Reducing makespan...");
    double maxSpeedup = -1;
    int taskToSpeedupId = -1;
    VMDescription vmToBuy;

    for (const View::Task& task: taskOrder) {
        VMDescription oldVM = taskVM[task.GetId()]; 
        VMDescription newVM = oldVM;
        for (const VMDescription& vm: viewer->GetVMList()) {
            if (task.CanBeExecuted(vm) &&
                vm > oldVM && 
                (newVM == oldVM || vm.GetPrice() <= newVM.GetPrice())) {
                newVM = vm;
            }
        }

        if (newVM != oldVM) {
            taskVM[task.GetId()] = newVM;

            double priceDiff = newVM.GetPrice() - oldVM.GetPrice();
            // priceDiff <= 0?
            double curSpeedup = (currentMakespan - CalculateMakespan(taskVM, taskOrder)) / priceDiff;

            if (curSpeedup > maxSpeedup) {
                maxSpeedup = curSpeedup;
                taskToSpeedupId = task.GetId();
                vmToBuy = newVM;
            }

            taskVM[task.GetId()] = oldVM;
        }
    }

    xbt_assert(taskToSpeedupId != -1, "Can't speedup current plan!");
    taskVM[taskToSpeedupId] = vmToBuy;
    
    XBT_INFO("Done!");
}

vector<double> MaoScheduler::CalculateTasksEndTimes(const vector<View::Task>& taskOrder,
                                                    const vector<VMDescription>& taskVM) {
    vector<double> tasksEndTimes(taskOrder.size(), -1);

    for (const View::Task& task: taskOrder) {
        double earliestBegin = 0;

        for (int dependencyId : task.GetDependencies()) {
            xbt_assert(tasksEndTimes[dependencyId] != -1, "Something went wrong, task order is inconsistent!");
            earliestBegin = max(earliestBegin, tasksEndTimes[dependencyId]);
        }
        tasksEndTimes[task.GetId()] = earliestBegin + task.GetExecutionTime(taskVM[task.GetId()]);
    }

    return tasksEndTimes;
}

vector<pair<double, double>> MaoScheduler::CalculateDeadlines(View::Viewer& v,
                                                              const vector<View::Task>& taskOrder,
                                                              const vector<VMDescription>& taskVM) {
    vector<double> tasksEndTimes = CalculateTasksEndTimes(taskOrder, taskVM);
    // for each time we construct a sorted list of all parents end times

    vector<vector<pair<int, double>>> endTimeParentList(taskOrder.size());
    
    for (const View::Task& task: taskOrder) {
        for (int dependencyId : task.GetDependencies()) {
            endTimeParentList[task.GetId()].push_back({dependencyId, tasksEndTimes[dependencyId]});
        }
    }

    for (auto& ancestorList: endTimeParentList) {
        sort(ancestorList.begin(), ancestorList.end(), [](const auto& a, const auto& b) {
            return a.second < b.second;
        });
    }

    vector<int> tasksEndTimeSorted(taskOrder.size());
    std::iota(tasksEndTimeSorted.begin(), tasksEndTimeSorted.end(), 0);
    sort(tasksEndTimeSorted.begin(), tasksEndTimeSorted.end(), [&tasksEndTimes](int a, int b) {
        return tasksEndTimes[a] > tasksEndTimes[b];
    });

    const pair<double, double> EMPTY_PAIR = {-1, -1};
    size_t calculatedDeadlines = 0;
    vector<pair<double, double>> deadlines(taskOrder.size(), EMPTY_PAIR);
    
    double totalDeadline = tasksEndTimes[tasksEndTimeSorted[0]];
    
    for (const int& endTaskId: tasksEndTimeSorted) {
        if (calculatedDeadlines == taskOrder.size()) {
            break;
        }

        while (!endTimeParentList[endTaskId].empty()) {
            // constructing new critical path
            vector<int> currentPath;
            int currentTaskId = endTaskId;
            double totalRuntime = 0;

            // last node is included in path iff its deadline is not calculated yet
            if (deadlines[currentTaskId] == EMPTY_PAIR) {
                currentPath.push_back(currentTaskId);
                totalRuntime += viewer->GetTaskById(currentTaskId).GetExecutionTime(taskVM[currentTaskId]);
            }

            do {
                if (endTimeParentList[currentTaskId].empty()) {
                    break;
                }
                int newTaskId = endTimeParentList[currentTaskId].back().first;
                endTimeParentList[currentTaskId].pop_back();

                currentTaskId = newTaskId;

                totalRuntime += viewer->GetTaskById(currentTaskId).GetExecutionTime(taskVM[currentTaskId]);

                currentPath.push_back(currentTaskId);
            } while (deadlines[currentTaskId] == EMPTY_PAIR);

            if (!currentPath.empty()) {
                double endTime = totalDeadline;
                if (deadlines[endTaskId] != EMPTY_PAIR) {
                    endTime = deadlines[endTaskId].first;
                }

                double beginTime = 0;
                if (deadlines[currentPath.back()] != EMPTY_PAIR) {
                    beginTime = deadlines[currentPath.back()].second;
                    totalRuntime -= viewer->GetTaskById(currentTaskId).GetExecutionTime(taskVM[currentTaskId]);
                    currentPath.pop_back();
                }

                reverse(currentPath.begin(), currentPath.end());

                double cumulativeTime = beginTime;
                for (const int& taskId: currentPath) {
                    xbt_assert(deadlines[taskId] == EMPTY_PAIR, "Task %d already has a deadline!", taskId);

                    double taskRuntime = viewer->GetTaskById(taskId).GetExecutionTime(taskVM[taskId]);
                    double taskRuntimeNorm = taskRuntime / totalRuntime;
                    
                    deadlines[taskId].first = cumulativeTime;
                    cumulativeTime += taskRuntimeNorm * (endTime - beginTime);
                    deadlines[taskId].second = cumulativeTime;

                    ++calculatedDeadlines;
                }
            }
        }
    }
    xbt_assert(deadlines.size() == viewer->WorkflowSize(), "Something went wrong, not all tasks have deadlines assigned!");

    for (pair<double, double>& deadline: deadlines) {
        deadline.first *= v.GetDeadline() / totalDeadline;
        deadline.second *= v.GetDeadline() / totalDeadline;
    }

    return deadlines;
}

vector<vector<LoadVectorEvent>> MaoScheduler::GetLoadVector(const vector<pair<double, double>>& deadlines,
                                                            const vector<VMDescription>& taskVM) {
    vector<vector<LoadVectorEvent>> loadVector(viewer->GetVMList().Size());
    for (int taskId = 0; taskId < static_cast<int>(deadlines.size()); ++taskId) {
        double runTime = viewer->GetTaskById(taskId).GetExecutionTime(taskVM[taskId]);
        double consumptionRatio = runTime / (deadlines[taskId].second - deadlines[taskId].first);
        int vmId = taskVM[taskId].GetId();
        loadVector[vmId].push_back({consumptionRatio, deadlines[taskId].first, deadlines[taskId].second, taskId, vmId});
    }
    return loadVector;
}
