#pragma once
#include "prototypes/interface.h"

enum ActionType {
    Abstract,
    AssignJob,
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

class AssignJobAction : AbstractAction {
    ScheduleItem Task;
    int HostId;

public:
    AssignJobAction(int hostId, int taskId);
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
    VMSpec Spec;
    BuyVMAction(VMSpec s):
        Spec(s)
        {}

    virtual void MakeAction(SimulatorInterface& interface) override;
    virtual ActionType GetActionType() const override;
}; 

