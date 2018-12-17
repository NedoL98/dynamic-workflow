#pragma once

#include <string>
#include <vector>

#include "tasks_graph.h"

class Scheduler {
public:
    Scheduler() = delete;
    Scheduler(int argc, char* argv[]);

    void ProcessTasksGraph(const TasksGraph& tasksGraph);

    void operator()();

private:
    std::vector<TasksGraph> tasksGraphs;
};