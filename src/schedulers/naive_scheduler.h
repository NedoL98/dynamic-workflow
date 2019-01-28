#pragma once

#include "scheduler_base.h"

class NaiveScheduler: public BaseScheduler {
public:
    NaiveScheduler() = delete;
    NaiveScheduler(const std::string& workflowPath, const std::string& vmListPath);

    void ProcessTasksGraph(TasksGraph& tasksGraph) override;
};