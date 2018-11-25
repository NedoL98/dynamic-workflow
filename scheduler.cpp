#include <simgrid/s4u.hpp>
#include <simgrid/plugins/live_migration.h>
#include <simgrid/s4u/VirtualMachine.hpp>

#include "scheduler.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(scheduler, "Scheduler log");

Scheduler::Scheduler(std::vector<std::string> args) {
    xbt_assert(args.size() > 1, "Scheduler expects number of tasks and their descriptions");

    TaskCount = std::stoi(args[1]);

    XBT_INFO("Reading description of %d tasks...", TaskCount);

    Tasks.resize(TaskCount);
    for (int i = 0; i < TaskCount; ++i) {
        Tasks[i] = Task(std::stoi(args[2 + i * 3]), std::stoi(args[3 + i * 3]), std::stoi(args[4 + i * 3]));
    }

    XBT_INFO("DONE!");
}

void Scheduler::operator()() {
    std::vector<simgrid::s4u::Host*> hosts = simgrid::s4u::Engine::get_instance()->get_all_hosts();

    for (int i = 0; i < TaskCount; i++) {
        // It seems that storing current number of free cores is not implemented 
        // so we should handle it ourselves
        simgrid::s4u::Host* host = hosts[rand() % hosts.size()];

        simgrid::s4u::VirtualMachine* vm = new simgrid::s4u::VirtualMachine("VM" + std::to_string(i), host, Tasks[i].GetCores());
        vm->start();
        Tasks[i].Execute(host);
    }
}