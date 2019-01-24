#pragma once

#include "scheduler_base.h"

class NaiveScheduler: public BaseScheduler {
public:
    NaiveScheduler() = delete;
    NaiveScheduler(const std::string& workflowPath);

    void ProcessTasksGraph(TasksGraph& tasksGraph) override;
};