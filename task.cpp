#include "task.h"

#include <exception>

XBT_LOG_NEW_DEFAULT_CATEGORY(task, "Task log");

Task::Task(double flops, int cores, int ram) 
    : Flops(flops)
    , Cores(cores)
    , Ram(ram)
    {}

std::string Task::GetName() {
    return Name;
}

std::map<std::string, std::string> Task::GetInputs() {
    return Inputs;
}

std::map<std::string, double> Task::GetOutputs() {
    return Outputs;
}

int Task::GetCores() {
    return Cores;
}

double Task::GetMemory() {
    return Ram;
}

double Task::GetSize() {
    return Flops;
}

void Task::SetName(const std::string& name) {
    Name = std::move(name);
}

void Task::AppendInput(const std::string& name, const std::string& source) {
    if (Inputs.count(name) > 0) {
        XBT_WARN("Input name is not unique! Previous input will be deleted!");
    }
    Inputs[name] = source;
}

void Task::AppendOutput(const std::string& name, const std::string& size) {
    if (Outputs.count(name) > 0) {
        XBT_WARN("Output name is not unique! Previous output will be deleted!");
    }
    Outputs[name] = ParseNumber(size, SizeSuffixes);
}

void Task::SetCores(const std::string& cores) {
    Cores = std::stoi(cores);
}

void Task::SetMemory(const std::string& memory) {
    Ram = std::stod(memory);
}

void Task::SetSize(const std::string& size) {
    Flops = ParseNumber(size, PerformanceSuffixes);
}

void Task::DoExecute(double flops) {
    double timeStart = simgrid::s4u::
    Engine::get_clock();
    simgrid::s4u::this_actor::execute(flops);
    double timeFinish = simgrid::s4u::Engine::get_clock();

    XBT_INFO("%s:%s task executed %g", simgrid::s4u::this_actor::get_host()->get_cname(),
            simgrid::s4u::this_actor::get_cname(), timeFinish - timeStart);
}

void Task::Execute(simgrid::s4u::Host* host) {
    simgrid::s4u::Actor::create("compute", host, DoExecute, Flops);
}