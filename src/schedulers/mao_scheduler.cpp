#include "mao_scheduler.h"

#include "tasks_graph.h"

using std::map;
using std::max;
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
}