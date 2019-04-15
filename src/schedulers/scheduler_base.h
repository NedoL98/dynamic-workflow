#pragma once

#include <string>
#include <vector>

#include "tasks_graph.h"
#include "vm_list.h"

class BaseScheduler {
public:
    BaseScheduler(const std::string& workflowPath, const std::string& vmListPath);

    virtual void ProcessTasksGraph() = 0;

    void GetMaxParams(int& hostCnt, int& maxCoresCnt, long long& maxMemory);

    void operator()();

protected:
    VMList VMList_;
    TasksGraph Workflow;
};
