#pragma once

#include <string>
#include <vector>

#include "../tasks_graph.h"
#include "../vm_list.h"

class BaseScheduler {
public:
    BaseScheduler() = delete;
    BaseScheduler(const std::string& workflowPath, const std::string& vmListPath);

    virtual void ProcessTasksGraph(TasksGraph& tasksGraph) = 0;

    void GetMaxParams(int& hostCnt, int& maxCoresCnt, double& maxMemory);

    void operator()();

private:
    VMList vmList;
    std::vector<TasksGraph> TasksGraphs;
};