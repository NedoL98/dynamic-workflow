#include "naive_scheduler.h"

NaiveScheduler::NaiveScheduler(int argc, char* argv[]) : BaseScheduler(argc, argv) {}

void NaiveScheduler::ProcessTasksGraph(TasksGraph& tasksGraph) {
    std::vector<std::shared_ptr<Task>> orderedTasks = tasksGraph.MakeTasksOrder();
    std::vector<simgrid::s4u::Host*> hosts = simgrid::s4u::Engine::get_instance()->get_all_hosts();

    std::map<std::string, simgrid::s4u::ActorPtr> actorPointers;
    std::map<std::string, simgrid::s4u::VirtualMachine*> vmPointers;

    std::set<std::string> processingTasks;

    for (const std::shared_ptr<Task>& task: orderedTasks) {
        for (std::string input: task->GetInputs()) {
            tasksGraph.Tasks[input]->Finish(actorPointers[input]);
            processingTasks.erase(input);
        }
        do {
            for (simgrid::s4u::Host* host: hosts) {
                if (task->CanExecute(host)) {
                    vmPointers[task->GetName()] = task->MakeVirtualMachine(host);
                    actorPointers[task->GetName()] = task->Execute(vmPointers[task->GetName()]);
                    processingTasks.insert(task->GetName());       
                    break;
                }
            }

            if (actorPointers[task->GetName()]) {
                break;
            }

            double minLoad = -1;
            std::string earliestToFinish;
            for (std::string processingTask: processingTasks) {
                double load = std::stod(actorPointers[processingTask]->get_property("size"));
                if (minLoad == -1 || load < minLoad) {
                    minLoad = load;
                    earliestToFinish = processingTask;
                }
            }

            tasksGraph.Tasks[earliestToFinish]->Finish(actorPointers[earliestToFinish]);
            processingTasks.erase(earliestToFinish);
        } while (!processingTasks.empty());
        xbt_assert(vmPointers[task->GetName()], "No host satisfies task %s requirements", task->GetName().c_str());
    }
}