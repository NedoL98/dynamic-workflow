#pragma once
#include "prototypes/scheduler.h"
#include "view/viewer.h"

class BaselineScheduler: public StaticScheduler {
public:
    virtual Actions PrepareForRun(View::Viewer& v) override;

    static AbstractScheduler* Create() { return new BaselineScheduler(); };

private:
    Actions MakeSchedule() const;
    VMDescription GetBestVM(const View::Task& task) const;
};