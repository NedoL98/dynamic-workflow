#pragma once

#include <iostream>

#include "task.h"
#include <yaml-cpp/yaml.h>

class Scheduler;

class TasksGraph {
friend class NaiveScheduler;

public:
    TasksGraph(std::string filepath);

    void MakeGraph();

    void MakeOrderDFS(const std::string& vertex, std::vector<std::shared_ptr<Task>>& order, std::map<std::string, bool>& used) const;
    std::vector<std::shared_ptr<Task>> MakeTasksOrder() const;

    int Size() const;
    int MaxCores() const;
    double MaxMemory() const;

    void PrintGraph() const;

private:
    std::string Name;

    //TODO: make appropriate interface
    std::map<std::string, double> Inputs;
    std::map<std::string, std::string> Outputs;

    std::map<std::string, std::shared_ptr<Task>> Tasks;

    std::map<std::string, int> OutputDegree;
    std::map<std::string, std::vector<std::string>> ReverseEdges;
};