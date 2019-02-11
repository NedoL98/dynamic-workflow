#pragma once

#include <iostream>

#include "task.h"
#include "vm_list.h"
#include <yaml-cpp/yaml.h>

class Scheduler;

class TasksGraph {
friend class NaiveScheduler;
friend class MaoScheduler;

public:
    TasksGraph() = default;
    TasksGraph(std::string filepath);

    void MakeGraph();
    void RemakeGraph(const std::vector<std::shared_ptr<Task>>& tasks);

    void MakeOrderDFS(const std::string& vertex, 
                      std::vector<std::shared_ptr<Task>>& order, 
                      std::map<std::string, bool>& used,
                      const std::map<std::string, std::vector<std::string>>& edges) const;
    std::vector<std::shared_ptr<Task>> MakeTasksOrder() const;

    double GetDeadline() const;
    int Size() const;
    int MaxCores() const;
    double MaxMemory() const;

    std::map<std::string, VMDescription> GetCheapestVMs(const VMList& vmList) const;

    int GetInputDegree(const std::string& taskName) const;
    int GetOutputDegree(const std::string& taskName) const;

    void PrintGraph() const;

private:
    std::string Name;
    double Deadline;

    //TODO: make appropriate interface
    std::map<std::string, double> Inputs;
    std::map<std::string, std::string> Outputs;

    std::map<std::string, std::shared_ptr<Task>> Tasks;

    std::map<std::string, int> InputDegree;
    std::map<std::string, int> OutputDegree;

    std::map<std::string, std::vector<std::string>> ReverseEdges;
};