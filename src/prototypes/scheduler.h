#pragma once
#include "schedule/schedule.h"
#include "schedule/schedule_action.h"
#include "events/event.h"
#include <vector>
#include <memory>

class AbstractScheduler {
public:
    typedef std::vector<std::shared_ptr<AbstractAction> > Actions;
    virtual Actions PrepareForRun() = 0;
    virtual Actions OnJobComplete(const JobFinishedEvent &e) = 0;
    virtual Actions OnJobFail(const JobFinishedEvent &e) = 0;
    virtual Actions OnActionComplete(const ActionCompletedEvent &event) = 0;
};

class StaticScheduler : public AbstractScheduler {

public:
    virtual Actions OnJobComplete(const JobFinishedEvent &e) override final;
    virtual Actions OnJobFail(const JobFinishedEvent &e) override final;
    virtual Actions OnActionComplete(const ActionCompletedEvent &event) override final;
};
