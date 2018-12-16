#pragma once

#include <iostream>

#include "task.h"

class TasksGraph {
public:
    TasksGraph(std::string filepath);

    void PrintGraph();

private:
    std::string Name;

    std::map<std::string, double> Inputs;
    std::map<std::string, std::string> Outputs;

    std::map<std::string, std::shared_ptr<Task>> Tasks;
};