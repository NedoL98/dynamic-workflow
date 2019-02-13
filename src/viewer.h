#pragma once
#include <prototypes/simulator.h>
#include <vector>
#include <string>
#include <spec.h>


class Viewer {
    const CloudSimulator& simulator;

public:
    Viewer(const CloudSimulator& s):
        simulator(s)
        {}
    
    int GetTaskId(const std::string& s) const;
    std::vector<int> GetTaskParents(int id) const;
    std::vector<int> GetTaskChildren(int id) const;
    std::string GetTaskName(int id) const;
    ComputeSpec GetTaskRequirements(int id) const;
    ComputeSpec GetTaskSize(int id) const;
};
