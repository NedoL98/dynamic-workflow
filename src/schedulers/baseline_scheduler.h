#pragma once
#include "prototypes/scheduler.h"
#include "view/viewer.h"

struct VMData {
    VMDescription VMDescr;
    std::vector<int> TasksQueue;
};

class BaselineScheduler: public StaticScheduler {
public:
    virtual Actions PrepareForRun(View::Viewer& v) override;

    static AbstractScheduler* Create() { return new BaselineScheduler(); };

private:
    void DoCalculateMakespanAndCost(const std::vector<VMData>& availableVMs, 
                                    const std::vector<int>& taskIdToVMId,
                                    const std::vector<int>& taskIdToVMPosition,
                                    double& makespan,
                                    double& cost) const;
    bool UpgradeRandomVM(std::vector<VMData>& availableVMs) const;

    BaselineScheduler::Actions MakeActions(const std::vector<VMData>& availableVMs) const;

    Actions MakeSchedule() const;
    VMDescription GetBestVM(const View::Task& task) const;
};