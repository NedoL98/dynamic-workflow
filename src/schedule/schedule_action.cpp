#include "schedule/schedule_action.h"

AssignTaskAction::AssignTaskAction(int hostId, int taskId):
    Task(taskId),
    HostId(hostId)
{}
    
void AssignTaskAction::MakeAction(SimulatorInterface& interface) {
    interface.AssignTask(HostId, Task);
}

ActionType AssignTaskAction::GetActionType() const {
    return ActionType::AssignTask;
}

ResetScheduleAction::ResetScheduleAction(const Schedule& s):
    S(s)
{}

void ResetScheduleAction::MakeAction(SimulatorInterface& interface) {
    interface.ResetSchedule(S);
}

ActionType ResetScheduleAction::GetActionType() const {
    return ActionType::ResetSchedule;
}

void BuyVMAction::MakeAction(SimulatorInterface& interface) {
    interface.RegisterVirtualMachine(Spec, CustomId);
}

ActionType BuyVMAction::GetActionType() const {
    return ActionType::BuyVM;
}
