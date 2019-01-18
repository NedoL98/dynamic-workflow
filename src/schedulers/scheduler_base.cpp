#include <simgrid/s4u.hpp>
#include <simgrid/plugins/live_migration.h>
#include <simgrid/s4u/VirtualMachine.hpp>

#include "scheduler_base.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(scheduler, "Scheduler log");

BaseScheduler::BaseScheduler(int argc, char* argv[]) {
    XBT_INFO("Loading tasks graphs...");
    // should probably redo this
    for (int i = 2; i < argc; ++i) {
        TasksGraphs.push_back(TasksGraph(argv[i]));
    }
    XBT_INFO("%d tasks graphs loaded", TasksGraphs.size());
}

void BaseScheduler::operator()() {
    for (TasksGraph& tasksGraph: TasksGraphs) {
        ProcessTasksGraph(tasksGraph);
    }
}