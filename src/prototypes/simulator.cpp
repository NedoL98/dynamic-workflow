#include "prototypes/simulator.h"
#include "view/viewer.h"

#include <memory>

void CloudSimulator::Run(double timeout) {
    View::Viewer v(*this);
    std::vector<std::shared_ptr<AbstractAction>> actions = Scheduler->PrepareForRun(&v);
    for (auto a : actions) {
        ProcessAction(a);
    }
}


bool CloudSimulator::RegisterVirtualMachine(const VMDescription &stats, int customId) {
    int hostId = -1;
    if ((hostId = Platform.GetEmptyHost(stats.GetSpec())) == -1) {
        return false;
    }
    return Platform.CreateVM(hostId, stats.GetSpec(), customId);
}

bool CloudSimulator::AssignTask(int VMId, const ScheduleItem &item) {
    if (!Platform.CheckTask(VMId, TaskSpec())) { // FIXME Get Task Spec
        return false;
    }
    Assignments.AddItem(VMId, item);
    return true;
}

bool CloudSimulator::CancelTask(int hostId, const ScheduleItem &item) {
    return false; // Not implemented
}

bool CloudSimulator::ResetSchedule(const Schedule& s) {
    Assignments = s;
    return true;
}
    
