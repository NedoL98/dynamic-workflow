#pragma once
#include "schedule/schedule.h"

class AbstractAction {
public:
    virtual void MakeAction(Schedule& s) = 0;
};

class AssignJobAction : AbstractAction {
    ScheduleItem Task;
    int HostId;

public:
    AssignJobAction(int hostId, int taskId);
    virtual void MakeAction(Schedule& s) override;
};

class ResetScheduleAction : AbstractAction {
    Schedule S;

public:
    ResetScheduleAction(const Schedule& s);
    virtual void MakeAction(Schedule& s) override;
};
