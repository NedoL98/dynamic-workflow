#pragma once

#include <iostream>

#include "task.h"
#include <yaml-cpp/yaml.h>

class Scheduler;

class TasksGraph {
friend class Scheduler;

public:
    TasksGraph(std::string filepath);

    void PrintGraph();

private:
    std::string Name;

    std::map<std::string, double> Inputs;
    std::map<std::string, std::string> Outputs;

    std::map<std::string, std::shared_ptr<Task>> Tasks;
};