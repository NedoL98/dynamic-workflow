#pragma once
#include "schedule/schedule.h"
#include "schedule/schedule_action.h"
#include <vector>
#include <memory>

class AbstractScheduler {
public:
    typedef std::vector<std::shared_ptr<AbstractAction> > Actions;
    virtual Actions PrepareForRun() = 0;
    virtual Actions OnJobComplete(int jobId) = 0;
    virtual Actions OnJobFail(int jobId) = 0;
};

class StaticScheduler : public AbstractScheduler {

public:
    virtual Actions OnJobComplete(int jobId) override final;
    virtual Actions OnJobFail(int jobId) override final;
};
