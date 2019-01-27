#include "schedule/schedule_action.h"

AssignJobAction::AssignJobAction(int hostId, int taskId):
    Task(taskId),
    HostId(hostId)
{}
    
void AssignJobAction::MakeAction(SimulatorInterface& interface) {
    interface.AssignJob(HostId, Task);
}

ActionType AssignJobAction::GetActionType() const {
    return ActionType::AssignJob;
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
