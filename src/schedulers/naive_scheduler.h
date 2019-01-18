#pragma once

#include "scheduler_base.h"

class NaiveScheduler: public BaseScheduler {
public:
    NaiveScheduler() = delete;
    NaiveScheduler(int argc, char* argv[]);

    void ProcessTasksGraph(TasksGraph& tasksGraph) override;
};