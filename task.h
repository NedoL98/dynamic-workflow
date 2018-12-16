#pragma once

#include <simgrid/s4u.hpp>

#include "common.h"

class Task {
public:
    Task() = default;
    
    Task(double flops, int cores, int ram);

    std::string GetName();
    std::map<std::string, std::string> GetInputs();
    std::map<std::string, double> GetOutputs();
    int GetCores();
    double GetMemory();
    double GetSize();

    void SetName(const std::string& name);
    void AppendInput(const std::string& name, const std::string& source);
    void AppendOutput(const std::string& name, const std::string& size);
    void SetCores(const std::string& cores);
    void SetMemory(const std::string& memory);
    void SetSize(const std::string& size);

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