#include "mao_scheduler.h"

#include "tasks_graph.h"

using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

MaoScheduler::MaoScheduler(const string& workflowPath, const string& vmListPath) : BaseScheduler(workflowPath, vmListPath) {}

void MaoScheduler::TasksBundling(const std::map<std::string, VMDescription>& taskVM) {
    vector<shared_ptr<Task>> taskOrder = Workflow.MakeTasksOrder();
}

void MaoScheduler::ProcessTasksGraph() {
    map<string, VMDescription> cheapestVM = Workflow.GetCheapestVMs(vmList);

    TasksBundling(cheapestVM);
}