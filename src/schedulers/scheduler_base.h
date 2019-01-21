#pragma once

#include <string>
#include <vector>

#include "../tasks_graph.h"

class BaseScheduler {
public:
    BaseScheduler() = delete;
    BaseScheduler(int argc, char* argv[]);

    virtual void ProcessTasksGraph(TasksGraph& tasksGraph) = 0;

    void GetMaxParams(int& hostCnt, int& maxCoresCnt, double& maxMemory);

    void operator()();

private:
    std::vector<TasksGraph> TasksGraphs;
};