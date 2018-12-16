#pragma once

#include <string>
#include <vector>

#include "tasks_graph.h"

class Scheduler {
public:
    Scheduler() = delete;
    Scheduler(std::vector<std::string> args);

    void operator()();

private:
    int TaskCount;
    std::vector<Task> Tasks;
};