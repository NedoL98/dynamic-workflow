#include <simgrid/s4u.hpp>
#include <simgrid/plugins/live_migration.h>
#include <simgrid/s4u/VirtualMachine.hpp>

#include "scheduler.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(scheduler, "Scheduler log");

Scheduler::Scheduler(int argc, char* argv[]) {
    XBT_INFO("Loading tasks graphs...");
    // should probably redo this
    for (int i = 2; i < argc; ++i) {
        tasksGraphs.push_back(TasksGraph(argv[i]));
    }
    XBT_INFO("%d tasks graphs loaded", tasksGraphs.size());
}

void Scheduler::ProcessTasksGraph(const TasksGraph& tasksGraph) {
    std::vector<simgrid::s4u::Host*> hosts = simgrid::s4u::Engine::get_instance()->get_all_hosts();
    std::cout << hosts.size() << std::endl;
    for (const auto& item: tasksGraph.Tasks) {
        std::cout << item.first << std::endl;
    }
}

void Scheduler::operator()() {
    for (const TasksGraph& tasksGraph: tasksGraphs) {
        ProcessTasksGraph(tasksGraph);
    }
    /*
    for (int i = 0; i < TaskCount; i++) {
        // It seems that storing current number of free cores is not implemented 
        // so we should handle it ourselves
        simgrid::s4u::Host* host = hosts[rand() % hosts.size()];

        simgrid::s4u::VirtualMachine* vm = new simgrid::s4u::VirtualMachine("VM" + std::to_string(i), host, Tasks[i].GetCores());
        vm->start();
        Tasks[i].Execute(host);
    }
    */
}