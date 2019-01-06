#include <simgrid/s4u.hpp>
#include <simgrid/plugins/live_migration.h>
#include <simgrid/s4u/VirtualMachine.hpp>

#include "scheduler.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(scheduler, "Scheduler log");

Scheduler::Scheduler(int argc, char* argv[]) {
    XBT_INFO("Loading tasks graphs...");
    // should probably redo this
    for (int i = 2; i < argc; ++i) {
        TasksGraphs.push_back(TasksGraph(argv[i]));
    }
    XBT_INFO("%d tasks graphs loaded", TasksGraphs.size());
}

void Scheduler::ProcessTasksGraph(TasksGraph& tasksGraph) {
    std::vector<std::shared_ptr<Task>> orderedTasks = tasksGraph.MakeTasksOrder();
    std::vector<simgrid::s4u::Host*> hosts = simgrid::s4u::Engine::get_instance()->get_all_hosts();

    std::map<std::string, simgrid::s4u::ActorPtr> actorPointers;
    std::map<std::string, simgrid::s4u::VirtualMachine*> vmPointers;

    for (const std::shared_ptr<Task>& task: orderedTasks) {
        for (std::string input: task->GetInputs()) {
            tasksGraph.Tasks[input]->Finish(actorPointers[input]);
        }
        for (simgrid::s4u::Host* host: hosts) {
            if (task->CanExecute(host)) {
                vmPointers[task->GetName()] = task->MakeVirtualMachine(host);
                actorPointers[task->GetName()] = task->Execute(vmPointers[task->GetName()]);       
                break;
            }
        }
        xbt_assert(vmPointers[task->GetName()], "No host satisfies task %s requirements", task->GetName().c_str());
    }
}

void Scheduler::operator()() {
    for (TasksGraph& tasksGraph: TasksGraphs) {
        ProcessTasksGraph(tasksGraph);
    }
}