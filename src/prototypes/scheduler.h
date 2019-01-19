#pragma once
#include <schedule/schedule.h>

class AbstractScheduler {
    Schedule schedule;

public:
    virtual void PrepareForRun() = 0;
    virtual void OnJobComplete(int jobId) = 0;
    virtual void OnJobFail(int jobId) = 0;
    Schedule GetSchedule();
};

class StaticScheduler : AbstractScheduler {

public:
    virtual void OnJobComplete(int jobId) delete;
    virtual void OnJobFail(int jobId) delete;
};
