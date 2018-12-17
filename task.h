#pragma once

#include <simgrid/s4u.hpp>
#include <yaml-cpp/yaml.h>

#include "common.h"

class Task {
public:
    Task() = default;
    
    Task(const YAML::Node& taskDescription);

    std::string GetName();
    std::map<std::string, std::string> GetInputs();
    std::map<std::string, double> GetOutputs();
    int GetCores();
    double GetMemory();
    double GetSize();

    void AppendInput(const std::string& name, const std::string& source);
    void AppendOutput(const std::string& name, const std::string& size);

    static void DoExecute(double flops);
    void Execute(simgrid::s4u::Host* host);

private:
    std::string Name;
    std::map<std::string, std::string> Inputs;
    std::map<std::string, double> Outputs;
    int Cores;
    double Ram;
    double Flops;
};