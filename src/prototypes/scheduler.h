#pragma once
#include <schedule/schedule.h>

class AbstractScheduler {
    Schedule schedule;

public:
    virtual void PrepareForRun() = 0;
    virtual void OnJobComplete(int jobId) = 0;
    virtual void OnJobFail(int jobId) = 0;
    Schedule GetSchedule() const;
};

class StaticScheduler : AbstractScheduler {

public:
    virtual void OnJobComplete(int jobId) override final;
    virtual void OnJobFail(int jobId) override final;
};
