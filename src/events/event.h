#pragma once
#include "schedule/schedule.h"
#include <memory>

enum EventStatus {
    Succeed,
    Failed
};

class AbstractEvent {
    EventStatus Status;

public:
    AbstractEvent(EventStatus s):
        Status(s) {
    }
    
    EventStatus GetStatus() const {
        return Status;
    }
};


class TaskFinishedEvent : public AbstractEvent {
    ScheduleItem Data;

public:
    TaskFinishedEvent(ScheduleItem data, EventStatus s):
        AbstractEvent(s),
        Data(data) {
    }

    ScheduleItem GetScheduleItem() const {
        return Data;
    }
};

class ActionCompletedEvent : public AbstractEvent {
    const std::shared_ptr<AbstractAction> Action;
    
public:
    ActionCompletedEvent(std::shared_ptr<AbstractAction> action, EventStatus s):
        AbstractEvent(s),
        Action(action) {
    }

    const std::shared_ptr<AbstractAction> GetAction() const {
        return Action;
    }
};
