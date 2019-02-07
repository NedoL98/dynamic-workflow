#pragma once

#include <simgrid/s4u.hpp>
#include <yaml-cpp/yaml.h>

#include "vm_list.h"
#include "common/common.h"

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

    void SetDefaultRequirements();

    void AppendRawInput(const std::string& name, const std::string& source);
    void AppendInput(const std::string& name);
    void AppendOutput(const std::string& name, const std::string& size);

    static void DoExecute(double flops, std::string name);
    simgrid::s4u::ActorPtr Execute(simgrid::s4u::VirtualMachine* vm);

    bool CanExecute(const simgrid::s4u::Host* host) const;
    bool CanExecute(const VMDescription& vmDescr) const;
    simgrid::s4u::VirtualMachine* MakeVirtualMachine(simgrid::s4u::Host* host);
    
    void ConsolidateTask(const Task& other);

    void Finish(simgrid::s4u::ActorPtr vmPtr);

private:
    std::string Name;
    std::map<std::string, std::string> RawInputs;
    std::vector<std::string> Inputs;
    std::map<std::string, double> Outputs;
    int Cores;
    double Memory;
    double Flops;

    simgrid::s4u::Host* Host = nullptr;
    bool Done = false;
};