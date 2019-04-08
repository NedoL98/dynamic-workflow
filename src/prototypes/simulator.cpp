#include "prototypes/simulator.h"
#include "view/viewer.h"
#include <simgrid/s4u.hpp>

#include <memory>

using std::vector;
XBT_LOG_NEW_DEFAULT_CATEGORY(simulator, "cloud simulator log");


void CloudSimulator::MainLoop(CloudSimulator* s) {
    s->DoMainLoop();    
}

int CloudSimulator::RefreshAfterTask(void* a, void* s) {
    ((CloudSimulator*)s)->DoRefreshAfterTask();
}

void CloudSimulator::DoRefreshAfterTask() {
    XBT_INFO("Refresh");
    vector<int> vmIds = Platform.GetVMIds();
    for (int vm : vmIds) {
        if (!Assignments.HasItem(vm)) {
            continue;
        }
        //XBT_INFO("%d task id, %d size", Assignments.GetItem(vm).GetTaskId(), TaskGraph.Nodes.size());
        auto task = *TaskGraph.Nodes[Assignments.GetItem(vm).GetTaskId()];
    }
}
void CloudSimulator::DoMainLoop() {
    
    XBT_INFO("MainLoop begins");
    vector<int> vmIds = Platform.GetVMIds();
    vector<simgrid::s4u::ActorPtr> Actors;
    for (int vm : vmIds) {
        if (!Assignments.HasItem(vm)) {
            continue;
        }
        auto task = *TaskGraph.Nodes[Assignments.GetItem(vm).GetTaskId()];
        if (task.GetDependencies().empty()) {
            Actors.push_back(Platform.AssignTask(vm, task.GetTaskSpec()));
            Actors.back()->on_exit(RefreshAfterTask, this);
        }
    }
    XBT_INFO("%d vms, %d actors", vmIds.size(), Actors.size());
    Actors[0]->join();
    
    XBT_INFO("MainLoop ends");
}

void CloudSimulator::Run(double timeout) {
    View::Viewer v(*this);
    
    simgrid::s4u::Engine* e = simgrid::s4u::Engine::get_instance();
    std::vector<std::shared_ptr<AbstractAction>> actions = Scheduler->PrepareForRun(v);
    XBT_INFO("initial actions processing");
    for (auto a : actions) {
        ProcessAction(a);
    }
    XBT_INFO("initial actions processing Done");
    simgrid::s4u::Actor::create("scheduler", e->get_all_hosts()[0], MainLoop, this);
    e->run();
}


bool CloudSimulator::RegisterVirtualMachine(const VMDescription &stats, int customId) {
    int hostId = -1;
    XBT_INFO("%d %f %f", stats.GetCores(), stats.GetFlops(), stats.GetMemory());
    if ((hostId = Platform.GetEmptyHost(stats.GetSpec())) == -1) {
        XBT_INFO("No empty hosts");
        return false;
    }
    return Platform.CreateVM(hostId, stats.GetSpec(), customId);
}

bool CloudSimulator::AssignTask(int VMId, const ScheduleItem &item) {
    if (!Platform.CheckTask(VMId, TaskGraph.Nodes[item.GetTaskId()]->GetTaskSpec())) {
        return false;
    }
    Assignments.AddItem(VMId, item);
    return true;
}

bool CloudSimulator::CancelTask(int hostId, const ScheduleItem &item) {
    return false; // Not implemented
}

bool CloudSimulator::ResetSchedule(const Schedule& s) {
    Assignments = s; // FIXME Check
    return true;
}
