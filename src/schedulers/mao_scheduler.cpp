#include "mao_scheduler.h"

#include "tasks_graph.h"

#include <numeric>

using std::map;
using std::max;
using std::pair;
using std::queue;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

XBT_LOG_NEW_DEFAULT_CATEGORY(mao_scheduler, "Mao scheduler log");

vector<VMDescription> MaoScheduler::GetCheapestVMs() {
    vector<VMDescription> cheapestVM;
    for (const View::Task& task: viewer->GetTaskList()) {
        int bestPrice = -1;
        ComputeSpec curSpec = task.GetComputeSpec();
        for (VMDescription vmDescr: viewer->GetAvailiableVMTaxes()) {
            // we probably want to reconsider choosing most efficient vm type for a task
            if (curSpec.Cores >= vmDescr.GetCores() &&
                curSpec.Memory >= vmDescr.GetMemory() &&
                (bestPrice == -1 || vmDescr.GetPrice() < bestPrice)) {
                bestPrice = vmDescr.GetPrice();
                cheapestVM.push_back(vmDescr);
            }
        }
    }
    return cheapestVM;
}

void MaoScheduler::MakeOrderDFS(int vertex, 
                                vector<View::Task>& order, 
                                vector<bool>& used) {
    used[vertex] = true;
    for (int i = 0; i < viewer->GetTaskById(vertex).GetSuccessors().size(); ++i) {
        if (!used[i]) {
            MakeOrderDFS(i, order, used);
        }
    }
    order.push_back(viewer->GetTaskById(vertex));
}

vector<View::Task> MaoScheduler::MakeTasksOrder() {
    vector<View::Task> order;

    vector<bool> used(viewer->GetTaskList().size());
    for (int i = 0; i < viewer->GetTaskList().size(); ++i) {
        if (!used[i] && viewer->GetTaskById(i).GetSuccessors().size() == 0) {
            MakeOrderDFS(i, order, used);
        }
    }
    xbt_assert(order.size() == viewer->GetTaskList().size(), "Something went wrong, not all tasks are included in tasks order!");
    
    return order;
}

// we should enhance this in future so that data transer time is considered
double CalculateMakespan(const vector<VMDescription>& taskVM, const vector<View::Task>& taskOrder) {
    double makespan = 0;

    vector<double> endTime(taskOrder.size(), -1);

    for (const View::Task& task: taskOrder) {
        double earliestBegin = 0;
        for (const View::Task& dependency: task.GetDependencies()) {
            xbt_assert(endTime[dependency.GetId()] != -1, "Something went wrong, task order is inconsistent!");
            earliestBegin = max(earliestBegin, endTime[dependency.GetId()]);
        }
        ComputeSpec taskSpec = task.GetComputeSpec();
        endTime[task.GetId()] = earliestBegin + (taskSpec.Speed / taskVM[task.GetId()].GetFlops());
        makespan = max(makespan, endTime[task.GetId()]);
    }

    return makespan;
}

void MaoScheduler::ReduceMakespan(vector<VMDescription>& taskVM, 
                                  const vector<View::Task>& taskOrder, 
                                  double currentMakespan) {
    XBT_INFO("Reducing makespan...");
    double maxSpeedup = 0;
    View::Task taskToSpeedup;
    VMDescription vmToBuy;

    for (const View::Task& task: taskOrder) {
        // const string taskName = elem.first;
        // shared_ptr<Task> task = elem.second;
        VMDescription oldVM = taskVM[task.GetId()]; 
        VMDescription newVM = oldVM;
        for (const VMDescription& vm: viewer->GetAvailiableVMTaxes()) {
            ComputeSpec taskSpec = task.GetComputeSpec();
            if (taskSpec.Cores >= vm.GetCores() &&
                taskSpec.Memory >= vm.GetMemory() &&
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
                taskToSpeedup = task;
                vmToBuy = newVM;
            }

            taskVM[task.GetId()] = oldVM;
        }
    }

    xbt_assert(maxSpeedup != 0, "Can't speedup current plan!");
    taskVM[taskToSpeedup.GetId()] = vmToBuy;
    
    XBT_INFO("Done!");
}

vector<double> MaoScheduler::CalculateTasksEndTimes(const vector<View::Task>& taskOrder,
                                                    const vector<VMDescription>& taskVM) {
    vector<double> tasksEndTimes(taskOrder.size(), -1);

    for (const View::Task& task: taskOrder) {
        double earliestBegin = 0;

        for (const View::Task& dependency: task.GetDependencies()) {
            xbt_assert(tasksEndTimes[dependency.GetId()] != -1, "Something went wrong, task order is inconsistent!");
            earliestBegin = max(earliestBegin, tasksEndTimes[dependency.GetId()]);
        }
        tasksEndTimes[task.GetId()] = earliestBegin + (task.GetComputeSpec().Speed / taskVM[task.GetId()].GetFlops());
    }

    return tasksEndTimes;
}

vector<pair<double, double>> MaoScheduler::CalculateDeadlines(const vector<View::Task>& taskOrder,
                                                              const vector<VMDescription>& taskVM) {
    vector<double> tasksEndTimes = CalculateTasksEndTimes(taskOrder, taskVM);
    // for each time we construct a sorted list of all parents end times

    vector<vector<pair<int, double>>> endTimeParentList;
    
    for (const View::Task& task: taskOrder) {
        for (const View::Task& dependency: task.GetDependencies()) {
            endTimeParentList[task.GetId()].push_back({dependency.GetId(), tasksEndTimes[dependency.GetId()]});
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
    vector<pair<double, double>> deadlines(taskOrder.size(), EMPTY_PAIR);
    
    double totalDeadline = tasksEndTimes[tasksEndTimeSorted[0]];
    
    for (const int& endTaskId: tasksEndTimeSorted) {
        if (deadlines.size() == taskOrder.size()) {
            break;
        }

        while (!endTimeParentList[endTaskId].empty()) {
            // constructing new critical path
            vector<int> currentPath;
            int currentTaskId = endTaskId;
            double totalRuntime = 0;

            // last node is included in path iff its deadline is not calculated yet
            if (deadlines[currentTaskId] != EMPTY_PAIR) {
                currentPath.push_back(currentTaskId);
                totalRuntime += viewer->GetTaskById(currentTaskId).GetComputeSpec().Speed / taskVM[currentTaskId].GetFlops();
            }

            do {
                if (endTimeParentList[currentTaskId].empty()) {
                    break;
                }
                int newTaskId = endTimeParentList[currentTaskId].back().first;
                endTimeParentList[currentTaskId].pop_back();

                currentTaskId = newTaskId;

                totalRuntime += viewer->GetTaskById(currentTaskId).GetComputeSpec().Speed / taskVM[currentTaskId].GetFlops();

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
                    totalRuntime -= viewer->GetTaskById(currentTaskId).GetComputeSpec().Speed / taskVM.at(currentPath.back()).GetFlops();
                    currentPath.pop_back();
                }

                reverse(currentPath.begin(), currentPath.end());

                double cumulativeTime = beginTime;
                for (const int& taskId: currentPath) {
                    xbt_assert(deadlines[taskId] == EMPTY_PAIR, "Task %d already has a deadline!", taskId);

                    double taskRuntime = viewer->GetTaskById(taskId).GetComputeSpec().Speed / taskVM[taskId].GetFlops(); 
                    double taskRuntimeNorm = taskRuntime / totalRuntime;
                    
                    deadlines[taskId].first = cumulativeTime;
                    cumulativeTime += taskRuntimeNorm * (endTime - beginTime);
                    deadlines[taskId].second = cumulativeTime;
                }
            }
        }
    }
    xbt_assert(deadlines.size() == viewer->GetTaskList().size(), "Something went wrong, not all tasks have deadlines assigned!");

    for (pair<double, double>& deadline: deadlines) {
        // FIXME when deadline can be get from viewer
        /*
        deadline.first *= Workflow.Deadline / totalDeadline;
        deadline.second *= Workflow.Deadline / totalDeadline;
        */
    }

    return deadlines;
}

vector<vector<LoadVectorEvent>> MaoScheduler::GetLoadVector(const vector<pair<double, double>>& deadlines,
                                                            const vector<VMDescription>& taskVM) {
    vector<vector<LoadVectorEvent>> loadVector(viewer->GetAvailiableVMTaxes().Size());
    for (int taskId = 0; taskId < deadlines.size(); ++taskId) {
        double runTime = viewer->GetTaskById(taskId).GetComputeSpec().Speed / taskVM[taskId].GetFlops();
        double consumptionRatio = runTime / (deadlines[taskId].second - deadlines[taskId].first);
        int vmId = taskVM[taskId].GetId();
        loadVector[vmId].push_back({consumptionRatio, deadlines[taskId].first, deadlines[taskId].second, taskId, vmId});
    }
    return loadVector;
}

MaoScheduler::Actions MaoScheduler::PrepareForRun(View::Viewer& v) {
    viewer = std::make_shared<View::Viewer>(v);

    vector<VMDescription> taskVM = GetCheapestVMs();

    // TasksBundling(taskVM);
    
    vector<View::Task> taskOrder = MakeTasksOrder();

    while (true) {
        double makespan = CalculateMakespan(taskVM, taskOrder);

        // // FIXME when deadline can be get from viewer
        /*        
        if (makespan <= ...) {
            break;
        }
        */

        XBT_INFO("Old makespan: %f", makespan);
        ReduceMakespan(taskVM, taskOrder, makespan);
        XBT_INFO("New makespan: %f", CalculateMakespan(taskVM, taskOrder));
    }

    vector<pair<double, double>> deadlines = CalculateDeadlines(taskOrder, taskVM);

    vector<vector<LoadVectorEvent>> loadVector = GetLoadVector(deadlines, taskVM);

    vector<LoadVectorEvent> sortedEvents;
    for (const auto& vmLoadVector: loadVector) {
        sortedEvents.insert(sortedEvents.end(), vmLoadVector.begin(), vmLoadVector.end());
    }
    std::sort(sortedEvents.begin(), sortedEvents.end(), [](const auto& a, const auto& b) {
        return a.End < b.End;
    });

    // TODO: make GetWorkflowSize method
    vector<simgrid::s4u::ActorPtr> actorPointers(viewer->GetTaskList().size());
    vector<bool> processingTasks(viewer->GetTaskList().size());

    for (const LoadVectorEvent& event: sortedEvents) {
        const View::Task task = viewer->GetTaskById(event.TaskId);

        for (const View::Task& dependency: task.GetDependencies()) {
            if (processingTasks[dependency.GetId()]) {
                /*
                Workflow.Tasks[input]->Finish(actorPointers[input]);
                processingTasks.erase(input);
                */
            }
        }

        XBT_INFO("Scheduling task %d", task.GetId());

        // we should reshedule task if its deadline can't be reached
        VMDescription currentVMDescription = taskVM[task.GetId()];
        /*
        simgrid::s4u::VirtualMachine* currentVMInstance = VMList_.GetVMInstance(task->GetName(), currentVMDescription.GetId());
        actorPointers[task->GetName()] = task->Execute(currentVMInstance, ActorFinishCallback, this);
        processingTasks.insert(task->GetName());
        */
    }

    XBT_INFO("Workflow processed!");

    return {};
}
