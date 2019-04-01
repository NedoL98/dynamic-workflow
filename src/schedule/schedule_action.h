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

class AssignTaskAction : public AbstractAction {
    ScheduleItem Task;
    int HostId;

public:
    AssignTaskAction(int hostId, int taskId);
    virtual void MakeAction(SimulatorInterface& interface) override;
    virtual ActionType GetActionType() const override;
};

class ResetScheduleAction : public AbstractAction {
    Schedule S;

public:
    ResetScheduleAction(const Schedule& s);
    virtual void MakeAction(SimulatorInterface& interface) override;
    virtual ActionType GetActionType() const override;
};

class BuyVMAction : public AbstractAction {
    VMDescription Spec;
    int CustomId;

public:
    BuyVMAction(VMDescription s, int customId):
        Spec(s),
        CustomId(customId)
        {}

    virtual void MakeAction(SimulatorInterface& interface) override;
    virtual ActionType GetActionType() const override;
}; 

