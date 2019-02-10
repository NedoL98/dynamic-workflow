#include "naive_scheduler.h"

#include <set>
#include <map>
#include <string>
#include <vector>

using std::set;
using std::map;
using std::string;
using std::vector;

NaiveScheduler::NaiveScheduler(const string& workflowPath, const string& vmListPath) : BaseScheduler(workflowPath, vmListPath) {}

void NaiveScheduler::ProcessTasksGraph() {
    vector<std::shared_ptr<Task>> orderedTasks = Workflow.MakeTasksOrder();
    vector<simgrid::s4u::Host*> hosts = simgrid::s4u::Engine::get_instance()->get_all_hosts();

    map<string, simgrid::s4u::ActorPtr> actorPointers;
    map<string, simgrid::s4u::VirtualMachine*> vmPointers;

    set<string> processingTasks;

    for (const std::shared_ptr<Task>& task: orderedTasks) {
        for (string input: task->GetInputs()) {
            Workflow.Tasks[input]->Finish(actorPointers[input]);
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
            string earliestToFinish;
            for (string processingTask: processingTasks) {
                double load = std::stod(actorPointers[processingTask]->get_property("size"));
                if (minLoad == -1 || load < minLoad) {
                    minLoad = load;
                    earliestToFinish = processingTask;
                }
            }

            Workflow.Tasks[earliestToFinish]->Finish(actorPointers[earliestToFinish]);
            processingTasks.erase(earliestToFinish);
        } while (!processingTasks.empty());
        xbt_assert(vmPointers[task->GetName()], "No host satisfies task %s requirements", task->GetName().c_str());
    }
}
