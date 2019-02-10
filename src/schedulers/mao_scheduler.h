#pragma once
#include "scheduler_base.h"

class MaoScheduler: public BaseScheduler {
public:
    MaoScheduler(const std::string& workflowPath, const std::string& vmListPath);

    void ProcessTasksGraph() override;

    double CalculateMakespan(const std::map<std::string, VMDescription>& taskVM);

    void TasksBundling(const std::map<std::string, VMDescription>& taskVM);
};