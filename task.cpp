#include "task.h"

#include <exception>

XBT_LOG_NEW_DEFAULT_CATEGORY(task, "Task log");

Task::Task(const YAML::Node& taskDescription) {
    xbt_assert(taskDescription["name"], "Task name is not specified!");
    Name = taskDescription["name"].as<std::string>();

    xbt_assert(taskDescription["inputs"], "Task inputs are not specified!");
    for (const YAML::Node& inputDescription: taskDescription["inputs"]) {
        xbt_assert(inputDescription["name"], "Input name is not specified!");
        std::string inputName = inputDescription["name"].as<std::string>();

        xbt_assert(inputDescription["source"], "Source for input is not specified!");
        std::string sourceName = inputDescription["source"].as<std::string>();

        AppendInput(inputName, sourceName);
    }

    xbt_assert(taskDescription["outputs"], "Task outputs are not specified!");
    for (const YAML::Node& outputDescription: taskDescription["outputs"]) {
        xbt_assert(outputDescription["name"], "Output name is not specified!");
        std::string outputName = outputDescription["name"].as<std::string>();

        xbt_assert(outputDescription["size"], "Output size is not specified!");
        std::string outputSize = outputDescription["size"].as<std::string>();

        AppendOutput(outputName, outputSize);
    }

    xbt_assert(taskDescription["requirements"], "Task requirements are not specified!");

    xbt_assert(taskDescription["requirements"]["cpu"], "CPU usage is not specified for task!");
    Cores = taskDescription["requirements"]["cpu"].as<int>();

    xbt_assert(taskDescription["requirements"]["memory"], "Memory usage is not specified for task!");
    Ram = ParseNumber(taskDescription["requirements"]["memory"].as<std::string>(), SizeSuffixes);

    xbt_assert(taskDescription["size"], "Task size is not specified!");
    Flops = ParseNumber(taskDescription["size"].as<std::string>(), PerformanceSuffixes);
}

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