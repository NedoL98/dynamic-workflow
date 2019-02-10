#include "mao_scheduler.h"

#include "tasks_graph.h"

using std::map;
using std::max;
using std::shared_ptr;
using std::string;
using std::vector;

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

double MaoScheduler::CalculateMakespan(const map<string, VMDescription>& taskVM) {
    vector<shared_ptr<Task>> taskOrder = Workflow.MakeTasksOrder();

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

void MaoScheduler::ProcessTasksGraph() {
    map<string, VMDescription> cheapestVM = Workflow.GetCheapestVMs(vmList);

    TasksBundling(cheapestVM);

    while (true) {
        double makespan = CalculateMakespan(cheapestVM);
        return;
    }
}