#include "mao_scheduler.h"

#include "tasks_graph.h"

using std::map;
using std::max;
using std::pair;
using std::queue;
using std::shared_ptr;
using std::string;
using std::vector;

XBT_LOG_NEW_DEFAULT_CATEGORY(mao_scheduler, "Mao scheduler log");

MaoScheduler::MaoScheduler(const string& workflowPath, const string& vmListPath) : BaseScheduler(workflowPath, vmListPath) {}

void MaoScheduler::TasksBundling(const std::map<std::string, VMDescription>& taskVM) {
    vector<shared_ptr<Task>> taskOrder = Workflow.MakeTasksOrder();

    int prevInd = 0;

    vector<shared_ptr<Task>> newTasks;
    newTasks.push_back(taskOrder[0]);

    for (int curInd = 1; curInd < static_cast<int>(taskOrder.size()); ++curInd) {
        // required and sufficient conditions to make tasks consolidation possible
        if (Workflow.GetOutputDegree(taskOrder[prevInd]->GetName()) == 1 &&
            Workflow.GetInputDegree(taskOrder[curInd]->GetName()) == 1 && 
            taskVM.at(taskOrder[prevInd]->GetName()) == taskVM.at(taskOrder[curInd]->GetName())) {
            taskOrder[prevInd]->ConsolidateTask(*taskOrder[curInd]);
        } else {
            newTasks.push_back(taskOrder[curInd]);
            prevInd = curInd;
        }
    }

    Workflow.RemakeGraph(newTasks);
}

// we should enhance this in future so that data transer time is considered
double MaoScheduler::CalculateMakespan(const map<string, VMDescription>& taskVM, const vector<shared_ptr<Task>>& taskOrder) {
    double makespan = 0;

    map<string, double> endTime;

    for (const auto task: taskOrder) {
        double earliestBegin = 0;
        for (const auto& input: task->GetInputs()) {
            xbt_assert(endTime.count(input), "Something went wrong, task order is inconsistent!");
            earliestBegin = max(earliestBegin, endTime[input]);
        }
        endTime[task->GetName()] = earliestBegin + (task->GetSize() / taskVM.at(task->GetName()).GetFlops());
        makespan = max(makespan, endTime.at(task->GetName()));
    }

    return makespan;
}

void MaoScheduler::ReduceMakespan(map<string, VMDescription>& taskVM, 
                                  const vector<shared_ptr<Task>>& taskOrder, 
                                  double currentMakespan) {
    XBT_INFO("Reducing makespan...");
    double maxSpeedup = 0;
    shared_ptr<Task> taskToSpeedup;
    VMDescription vmToBuy;

    for (auto& elem: Workflow.Tasks) {
        const string taskName = elem.first;
        shared_ptr<Task> task = elem.second;
        VMDescription oldVM = taskVM.at(taskName); 
        VMDescription newVM = taskVM.at(taskName);
        for (const auto& vm: vmList) {
            if (task->CanExecute(vm) && vm > oldVM && (newVM == oldVM || vm.GetPrice() <= newVM.GetPrice())) {
                newVM = vm;
            }
        }

        if (newVM != oldVM) {
            taskVM[taskName] = newVM;

            double priceDiff = newVM.GetPrice() - oldVM.GetPrice();
            // priceDiff <= 0?
            double curSpeedup = (currentMakespan - CalculateMakespan(taskVM, taskOrder)) / priceDiff;

            if (curSpeedup > maxSpeedup) {
                maxSpeedup = curSpeedup;
                taskToSpeedup = task;
                vmToBuy = newVM;
            }

            taskVM[taskName] = oldVM;
        }
    }

    xbt_assert(maxSpeedup != 0, "Can't speedup current plan!");
    taskVM[taskToSpeedup->GetName()] = vmToBuy;
    
    XBT_INFO("Done!");
}

map<string, double> MaoScheduler::CalculateTasksEndTimes(const vector<shared_ptr<Task>>& taskOrder,
                                                         const map<string, VMDescription>& taskVM) const {
    map<string, double> tasksEndTimes;

    for (const auto& task: taskOrder) {
        double earliestBegin = 0;

        for (const auto& input: task->GetInputs()) {
            xbt_assert(tasksEndTimes.count(input), "Something went wrong, task order is inconsistent!");
            earliestBegin = max(earliestBegin, tasksEndTimes[input]);
        }
        tasksEndTimes[task->GetName()] = earliestBegin + (task->GetSize() / taskVM.at(task->GetName()).GetFlops());
    }

    return tasksEndTimes;
}

map<string, pair<double, double>> MaoScheduler::CalculateDeadlines(const vector<shared_ptr<Task>>& taskOrder,
                                                                   const map<string, VMDescription>& taskVM) const {
    map<string, double> tasksEndTimes = CalculateTasksEndTimes(taskOrder, taskVM);
    // for each time we construct a sorted list of all parents end times
    map<string, vector<pair<string, double>>> endTimeParentList;
    
    for (const auto& [taskName, task]: Workflow.Tasks) {
        for (const auto& input: task->GetInputs()) {
            endTimeParentList[taskName].push_back({Workflow.Tasks.at(input)->GetName(), tasksEndTimes.at(input)});
        }
    }

    for (auto& [taskName, ancestorList]: endTimeParentList) {
        sort(ancestorList.begin(), ancestorList.end(), [](const auto& a, const auto& b) {
            return a.second < b.second;
        });
    }

    vector<string> tasksEndTimeSorted; 
    std::transform(tasksEndTimes.begin(), 
                   tasksEndTimes.end(), 
                   std::back_inserter(tasksEndTimeSorted), 
                   [](auto &elem) { return elem.first; });
    sort(tasksEndTimeSorted.begin(), tasksEndTimeSorted.end(), [&tasksEndTimes](const string& a, const string& b) {
        return tasksEndTimes.at(a) > tasksEndTimes.at(b);
    });

    map<string, pair<double, double>> deadlines;
    
    double totalDeadline = tasksEndTimes[tasksEndTimeSorted[0]];
    
    for (const string& endTaskName: tasksEndTimeSorted) {
        if (deadlines.size() == Workflow.Tasks.size()) {
            break;
        }

        while (!endTimeParentList[endTaskName].empty()) {
            // constructing new critical path
            vector<string> currentPath;
            string currentTaskName = endTaskName;
            double totalRuntime = 0;

            // last node is included in path iff its deadline is not calculated yet
            if (deadlines.count(currentTaskName) == 0) {
                currentPath.push_back(currentTaskName);
                totalRuntime += Workflow.Tasks.at(currentTaskName)->GetSize() / taskVM.at(currentTaskName).GetFlops();
            }

            do {
                if (endTimeParentList[currentTaskName].empty()) {
                    break;
                }
                string newTaskName = endTimeParentList[currentTaskName].back().first;
                endTimeParentList[currentTaskName].pop_back();

                currentTaskName = newTaskName;

                totalRuntime += Workflow.Tasks.at(currentTaskName)->GetSize() / taskVM.at(currentTaskName).GetFlops();

                currentPath.push_back(currentTaskName);
            } while (deadlines.count(currentTaskName) == 0);

            if (!currentPath.empty()) {
                double endTime = totalDeadline;
                if (deadlines.count(endTaskName) != 0) {
                    endTime = deadlines[endTaskName].first;
                }

                double beginTime = 0;
                if (deadlines.count(currentPath.back()) != 0) {
                    beginTime = deadlines[currentPath.back()].second;
                    totalRuntime -= Workflow.Tasks.at(currentPath.back())->GetSize() / taskVM.at(currentPath.back()).GetFlops();
                    currentPath.pop_back();
                }

                reverse(currentPath.begin(), currentPath.end());

                double cumulativeTime = beginTime;
                for (const string& taskName: currentPath) {
                    xbt_assert(deadlines.count(taskName) == 0, "Task %s already has a deadline!", taskName);

                    double taskRuntime = Workflow.Tasks.at(taskName)->GetSize() / taskVM.at(taskName).GetFlops(); 
                    double taskRuntimeNorm = taskRuntime / totalRuntime;
                    
                    deadlines[taskName].first = cumulativeTime;
                    cumulativeTime += taskRuntimeNorm * (endTime - beginTime);
                    deadlines[taskName].second = cumulativeTime;
                }
            }
        }
    }
    xbt_assert(deadlines.size() == Workflow.Tasks.size(), "Something went wrong, not all tasks have deadlines assigned!");

    for (auto& [taskName, deadline]: deadlines) {
        deadline.first *= Workflow.Deadline / totalDeadline;
        deadline.second *= Workflow.Deadline / totalDeadline;
    }

    return deadlines;
}

vector<vector<LoadVectorEvent>> MaoScheduler::GetLoadVector(const map<string, pair<double, double>>& deadlines,
                                                            const map<string, VMDescription>& taskVM) {
    vector<vector<LoadVectorEvent>> loadVector(vmList.Size());
    for (const auto& [taskName, deadline]: deadlines) {
        double runTime = Workflow.Tasks.at(taskName)->GetSize() / taskVM.at(taskName).GetFlops();
        double consumptionRatio = runTime / (deadline.second - deadline.first);
        loadVector[taskVM.at(taskName).GetId()].push_back({consumptionRatio, deadline.first, deadline.second});
    }
    return loadVector;
}

void MaoScheduler::ProcessTasksGraph() {
    map<string, VMDescription> taskVM = Workflow.GetCheapestVMs(vmList);

    TasksBundling(taskVM);

    vector<shared_ptr<Task>> taskOrder = Workflow.MakeTasksOrder();

    while (true) {
        double makespan = CalculateMakespan(taskVM, taskOrder);

        if (makespan <= Workflow.GetDeadline()) {
            break;
        }

        XBT_INFO("Old makespan: %f", makespan);
        ReduceMakespan(taskVM, taskOrder, makespan);
        XBT_INFO("New makespan: %f", CalculateMakespan(taskVM, taskOrder));
    }

    map<string, pair<double, double>> deadlines = CalculateDeadlines(taskOrder, taskVM);

    vector<vector<LoadVectorEvent>> loadVector = GetLoadVector(deadlines, taskVM);
}
