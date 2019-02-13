#pragma once
#include "schedule/schedule.h"
#include "schedule/schedule_action.h"
#include "events/event.h"
#include <vector>
#include <memory>

class Viewer;

class AbstractScheduler {
    std::shared_ptr<Viewer> viewer;
public:
    typedef std::vector<std::shared_ptr<AbstractAction> > Actions;
    virtual Actions PrepareForRun(Viewer *v) = 0;
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
