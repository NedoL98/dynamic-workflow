#pragma once
#include "prototypes/interface.h"

enum ActionType {
    Abstract,
    AssignTask,
    ResetSchedule,
    BuyVM
};

class AbstractAction {
public:
    virtual void MakeAction(SimulatorInterface& interface) = 0;
    virtual ActionType GetActionType() const {
        return ActionType::Abstract;
    }
};

class AssignTaskAction : AbstractAction {
    ScheduleItem Task;
    int HostId;

public:
    AssignTaskAction(int hostId, int taskId);
    virtual void MakeAction(SimulatorInterface& interface) override;
    virtual ActionType GetActionType() const override;
};

class ResetScheduleAction : AbstractAction {
    Schedule S;

public:
    ResetScheduleAction(const Schedule& s);
    virtual void MakeAction(SimulatorInterface& interface) override;
    virtual ActionType GetActionType() const override;
};

class BuyVMAction : AbstractAction {
    VMDescription Spec;
    BuyVMAction(VMDescription s):
        Spec(s)
        {}

    virtual void MakeAction(SimulatorInterface& interface) override;
    virtual ActionType GetActionType() const override;
}; 

