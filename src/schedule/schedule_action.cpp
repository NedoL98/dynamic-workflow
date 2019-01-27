#include "schedule/schedule_action.h"

AssignJobAction::AssignJobAction(int hostId, int taskId):
    Task(taskId),
    HostId(hostId)
{}
    
void AssignJobAction::MakeAction(Schedule& s) {
    s.AddItem(HostId, Task);
}

ResetScheduleAction::ResetScheduleAction(const Schedule& s):
    S(s)
{}

void ResetScheduleAction::MakeAction(Schedule& s) {
    s = S;
}
