#pragma once

#include <simgrid/s4u.hpp>
#include <yaml-cpp/yaml.h>

#include "common.h"

class Task {
public:
    Task() = default;
    
    Task(const YAML::Node& taskDescription);

    const std::string& GetName() const;
    const std::map<std::string, std::string>& GetRawInputs() const;
    const std::vector<std::string>& GetInputs() const;
    const std::map<std::string, double>& GetOutputs() const;
    int GetCores() const;
    double GetMemory() const;
    double GetSize() const;

    void AppendRawInput(const std::string& name, const std::string& source);
    void AppendInput(const std::string& name);
    void AppendOutput(const std::string& name, const std::string& size);
    void MarkAsDone();

    static void DoExecute(double flops, std::string name);
    simgrid::s4u::ActorPtr Execute(simgrid::s4u::Host* host);

private:
    std::string Name;
    std::map<std::string, std::string> RawInputs;
    std::vector<std::string> Inputs;
    std::map<std::string, double> Outputs;
    int Cores;
    double Ram;
    double Flops;

    bool Done = false;
};