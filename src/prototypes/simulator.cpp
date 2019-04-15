#include "prototypes/simulator.h"
#include "view/viewer.h"
#include <simgrid/s4u.hpp>

#include <memory>

using std::vector;
XBT_LOG_NEW_DEFAULT_CATEGORY(simulator, "cloud simulator log");


void CloudSimulator::MainLoop(CloudSimulator* s) {
    s->DoMainLoop();    
}

int CloudSimulator::RefreshAfterTask(int, void* s) {
    CollbackData* args = (CollbackData* )s;
    args->Simulator->DoRefreshAfterTask(args->TaskId);
    delete args;
    return 0;
}

simgrid::s4u::MutexPtr mutex;
void CloudSimulator::CheckReadyJobs() {
    // mutex->lock();

    vector<int> vmIds = Platform.GetVMIds();
    for (int vm : vmIds) {
        if (!Assignments.HasItem(vm)) {
            continue;
        }
        //XBT_INFO("%d task id, %d size", Assignments.GetItem(vm).GetTaskId(), TaskGraph.Nodes.size());
        int taskId = Assignments.GetItem(vm).GetTaskId();
        if (TaskGraph.Nodes[taskId]->IsReady()) {
            XBT_DEBUG("Task %d is ready to compute!", taskId);
            if (!TaskGraph.Nodes[taskId]->StartExecuting()) {
                XBT_INFO("Task %d isn't executing!", taskId);
                continue;
            }
            CollbackData* data = new CollbackData();
            data->Simulator = this;
            data->TaskId = taskId;
            Actors.push_back(Platform.AssignTask(vm, TaskGraph.Nodes[taskId]->GetTaskSpec(), RefreshAfterTask, data));
            XBT_DEBUG("Task %d started executing!", taskId);
        }
    }
    // mutex->unlock();
}

void CloudSimulator::DoRefreshAfterTask(int taskId) {

    TaskGraph.FinishTask(taskId);
    int hostId = Assignments.GetHostByTask(taskId);
    if (Assignments.GetItem(hostId).GetTaskId() == taskId) {
        Assignments.PopItem(hostId);
    }
    CheckReadyJobs();
    XBT_DEBUG("Task %d finished executing!", taskId);
}
void CloudSimulator::DoMainLoop() {
    
    XBT_INFO("MainLoop begins");
    CheckReadyJobs();
    XBT_INFO("%d vms, %d actors", Platform.GetVMIds().size(), Actors.size());
    for (size_t i = 0; i < Actors.size(); i++) {
        Actors[i]->join();
        //CheckReadyJobs();
    }
    xbt_assert(TaskGraph.IsFinished(), "All actors finished their work, but not all tasks done");
    
    XBT_INFO("MainLoop ends");
}

void CloudSimulator::Run(double timeout) {
    View::Viewer v(*this);
    mutex = simgrid::s4u::Mutex::create();   
    simgrid::s4u::Engine* e = simgrid::s4u::Engine::get_instance();
    std::vector<std::shared_ptr<AbstractAction>> actions = Scheduler->PrepareForRun(v);
    XBT_INFO("initial actions processing");
    for (auto a : actions) {
        ProcessAction(a);
    }
    XBT_INFO("initial actions processing Done");
    simgrid::s4u::Actor::create("scheduler", e->get_all_hosts()[0], MainLoop, this);
    e->run();
    XBT_INFO("required timeout is %g", timeout);
}


bool CloudSimulator::RegisterVirtualMachine(const VMDescription &stats, int customId) {
    int hostId = -1;
    XBT_INFO("%d %f %f", stats.GetCores(), stats.GetFlops(), stats.GetMemory());
    if ((hostId = Platform.GetEmptyHost(stats.GetSpec())) == -1) {
        XBT_INFO("No empty hosts");
        return false;
    }
    return Platform.CreateVM(hostId, stats, customId);
}

bool CloudSimulator::AssignTask(int VMId, const ScheduleItem &item) {
    if (!Platform.CheckTask(VMId, TaskGraph.Nodes[item.GetTaskId()]->GetTaskSpec())) {
        return false;
    }
    Assignments.AddItem(VMId, item);
    return true;
}

bool CloudSimulator::CancelTask(int hostId, const ScheduleItem &item) {
    (void)hostId;
    (void)item;
    return false; // Not implemented
}

bool CloudSimulator::ResetSchedule(const Schedule& s) {
    Assignments = s; // FIXME Check
    return true;
}
