#include <simgrid/s4u.hpp>
#include <simgrid/plugins/live_migration.h>
#include <simgrid/s4u/VirtualMachine.hpp>

#include "scheduler_base.h"

using std::max;
using std::string;

XBT_LOG_NEW_DEFAULT_CATEGORY(scheduler, "Scheduler log");

BaseScheduler::BaseScheduler(const string& workflowPath, const string& vmListPath) {
    XBT_INFO("Loading tasks graph...");
    Workflow = TasksGraph(workflowPath);
    XBT_INFO("Tasks graph loaded");

    XBT_INFO("Loading VM list...");
    VMList_ = VMList(vmListPath);
    XBT_INFO("VM list loaded");
}

void BaseScheduler::GetMaxParams(int& hostCnt, int& maxCoresCnt, long long& maxMemory) {
    hostCnt = Workflow.Size();
    maxCoresCnt = max(Workflow.MaxCores(), VMList_.MaxCores());
    maxMemory = max(Workflow.MaxMemory(), VMList_.MaxMemory());
}

void BaseScheduler::operator()() {
    ProcessTasksGraph();
}
