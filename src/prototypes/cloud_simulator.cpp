#include "prototypes/cloud_simulator.h"
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

void CloudSimulator::DoRefreshAfterTransfer(TransferSpec* spec) {
    if (!spec) {
        XBT_WARN("Something went wrong!");
    }
    if (Transfers.count(spec)) {
        Transfers.erase(spec);
        TaskGraph->FinishTransfer(*spec);
        Scheduler->OnTransferFinished(GenerateEvent<TransferFinishedEvent>(EventStatus::Succeed, spec->FileId, spec->Sender, spec->Receiver));
        CheckReadyJobOnVM(spec->Receiver);
        delete spec;
    } else if (spec->Sender == spec->Receiver) {
        TaskGraph->FinishTransfer(*spec);
        CheckReadyJobOnVM(spec->Receiver);
    } else {
        Transfers.insert(spec);
    }
}

void CloudSimulator::CheckReadyJobs() {
    vector<int> vmIds = Platform->GetVMIds();
    for (int vm : vmIds) {
        CheckReadyJobOnVM(vm);
    }
}

void CloudSimulator::CheckReadyFiles() {
    static simgrid::s4u::MutexPtr mutex = simgrid::s4u::Mutex::create();
    mutex->lock();
    auto iterator = TaskGraph->GetReadyFilesIterator();
    while (iterator) {
        auto file = *iterator;
        XBT_INFO("File %s is ready", file->Name.c_str());
        iterator++;
        for (int receiver : file->Receivers) {
            if (receiver == -1) {
                continue;
            }
            int hostSender = Assignments.GetHostByTask(file->Author);
            int hostReceiver = Assignments.GetHostByTask(receiver);
            XBT_INFO("File sender: %d on host %d, receiver: %d on host %d", file->Author, hostSender, receiver, hostReceiver);

            TransferSpec spec({file->Size, hostSender, hostReceiver, receiver, file->Id});
            Platform->StartTransfer(spec);
            Scheduler->OnTransferStarted(GenerateEvent<TransferStartedEvent>(EventStatus::Succeed, file->Id, hostSender, hostReceiver));
        }
        TaskGraph->StartTransfer(file->Id);
    }
    mutex->unlock();
}
void CloudSimulator::CheckReadyJobOnVM(int vm) {
    if (!Assignments.HasItem(vm)) {
        return;
    }
    int taskId = Assignments.GetItem(vm).GetTaskId();
    if (TaskGraph->GetTask(taskId).IsReady()) {
        XBT_DEBUG("Task %d is ready to compute!", taskId);
        if (!TaskGraph->GetTask(taskId).StartExecuting()) {
            XBT_INFO("Task %d isn't executing!", taskId);
            return;
        }
        CollbackData* data = new CollbackData();
        data->Simulator = this;
        data->TaskId = taskId;
        Actors.push_back(Platform->AssignTask(vm, TaskGraph->GetTask(taskId).GetTaskSpec(), RefreshAfterTask, data));
        Scheduler->OnTaskStarted(GenerateEvent<TaskStartedEvent>(EventStatus::Succeed, taskId, vm));
        XBT_DEBUG("Task %d started executing!", taskId);
    }
}

void CloudSimulator::DoRefreshAfterTask(int taskId) { // call on each task finishing
    int hostId = Assignments.GetHostByTask(taskId);
    Scheduler->OnTaskComplete(GenerateEvent<TaskFinishedEvent>(EventStatus::Succeed, taskId, hostId));
    TaskGraph->FinishTask(taskId);
    Platform->FinishTask(hostId, TaskGraph->GetTask(taskId).GetTaskSpec());
    if (Assignments.GetItem(hostId).GetTaskId() == taskId) {
        Assignments.PopItem(hostId);
    }
    CheckReadyJobs();
    CheckReadyFiles();
    XBT_DEBUG("Task %d finished executing!", taskId);
}
void CloudSimulator::DoMainLoop() {
    XBT_INFO("MainLoop begins");
    CheckReadyJobs();
    XBT_INFO("%d vms, %d actors", Platform->GetVMIds().size(), Actors.size());
    while (!TaskGraph->IsFinished()) {
        simgrid::s4u::this_actor::sleep_for(1);
        for (size_t i = 0; i < Actors.size(); i++) {
            Actors[i]->join();
        }
    }
    xbt_assert(TaskGraph->IsFinished(), "All actors finished their work, but not all tasks done");
    
    XBT_INFO("MainLoop ends");
    delete (CloudPlatform*)Platform;
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
    XBT_INFO("required timeout is %g", timeout);
}

// Inherited from Interface
bool CloudSimulator::RegisterVirtualMachine(const VMDescription &stats, int customId) {
    int hostId = -1;
    XBT_DEBUG("%d %lld %lld", stats.GetCores(), stats.GetFlops(), stats.GetMemory());
    if ((hostId = dynamic_cast<CloudPlatform* >(Platform)->GetEmptyHost(stats.GetSpec())) == -1) {
        XBT_INFO("No empty hosts");
        return false;
    }
    return Platform->CreateVM(hostId, stats, customId);
}

bool CloudSimulator::AssignTask(int VMId, const ScheduleItem &item) {
    if (!dynamic_cast<CloudPlatform* >(Platform)->CheckTask(VMId, TaskGraph->GetTask(item.GetTaskId()).GetTaskSpec())) {
        XBT_WARN("Schedule is invalid: %d task cannot be hosted on %d!", item.GetTaskId(), VMId);
        return false;
    }
    Assignments.AddItem(VMId, item);
    TaskGraph->AssignTask(item.GetTaskId(), VMId);
    return true;
}

bool CloudSimulator::CancelTask(int hostId, const ScheduleItem &item) {
    (void)hostId;
    (void)item;
    return false; // FIXME Not implemented
}

bool CloudSimulator::ResetSchedule(const Schedule& s) {
    Schedule copy = s;
    vector<int> vmIds = Platform->GetVMIds();
    Assignments = Schedule();
    for (int v : vmIds) {
        while (copy.HasItem(v)) {
            auto taskItem = copy.PopItem(v);
            AssignTask(v, taskItem);
        }
    }
    return true;
}
