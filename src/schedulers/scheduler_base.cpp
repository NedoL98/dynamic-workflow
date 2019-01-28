#include <simgrid/s4u.hpp>
#include <simgrid/plugins/live_migration.h>
#include <simgrid/s4u/VirtualMachine.hpp>

#include "scheduler_base.h"

using std::string;

XBT_LOG_NEW_DEFAULT_CATEGORY(scheduler, "Scheduler log");

BaseScheduler::BaseScheduler(const string& workflowPath, const string& vmListPath) {
    XBT_INFO("Loading tasks graph...");
    // should probably redo this
    TasksGraphs.push_back(TasksGraph(workflowPath));
    XBT_INFO("%d tasks graphs loaded", TasksGraphs.size());

    XBT_INFO("Loading VM list...");
    vmList = VMList(vmListPath);
    XBT_INFO("VM list loaded");
}

void BaseScheduler::GetMaxParams(int& hostCnt, int& maxCoresCnt, double& maxMemory) {
    hostCnt = 0;
    maxCoresCnt = 0;
    maxMemory = 0;
    for (const auto& tasksGraph: TasksGraphs) {
        hostCnt = std::max(hostCnt, tasksGraph.Size());
        maxCoresCnt = std::max(maxCoresCnt, tasksGraph.MaxCores());
        maxMemory = std::max(maxMemory, tasksGraph.MaxMemory());
    }
}

void BaseScheduler::operator()() {
    for (TasksGraph& tasksGraph: TasksGraphs) {
        ProcessTasksGraph(tasksGraph);
    }
}