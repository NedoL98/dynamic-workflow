#include "task.h"

#include <exception>
#include <iostream>

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

        AppendRawInput(inputName, sourceName);

        std::size_t delimPosition = sourceName.find('.');
        if (delimPosition != std::string::npos) {
            AppendInput(sourceName.substr(0, delimPosition));
        }
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

const std::string& Task::GetName() const {
    return Name;
}

const std::map<std::string, std::string>& Task::GetRawInputs() const {
    return RawInputs;
}

const std::vector<std::string>& Task::GetInputs() const {
    return Inputs;
}

const std::map<std::string, double>& Task::GetOutputs() const {
    return Outputs;
}

int Task::GetCores() const {
    return Cores;
}

double Task::GetMemory() const {
    return Ram;
}

double Task::GetSize() const {
    return Flops;
}

void Task::AppendRawInput(const std::string& name, const std::string& source) {
    if (RawInputs.count(name) > 0) {
        XBT_WARN("Input name is not unique! Previous input will be deleted!");
    }
    RawInputs[name] = source;
}

void Task::AppendInput(const std::string& name) {
    Inputs.push_back(name);
}

void Task::AppendOutput(const std::string& name, const std::string& size) {
    if (Outputs.count(name) > 0) {
        XBT_WARN("Output name is not unique! Previous output will be deleted!");
    }
    Outputs[name] = ParseNumber(size, SizeSuffixes);
}

void Task::MarkAsDone() {
    Done = true;
}

void Task::DoExecute(double flops, std::string name) {
    double timeStart = simgrid::s4u::Engine::get_clock();
    simgrid::s4u::this_actor::get_host()->execute(flops);
    double timeFinish = simgrid::s4u::Engine::get_clock();

    XBT_INFO("%s: %s task %s executed %g", simgrid::s4u::this_actor::get_host()->get_cname(),
            simgrid::s4u::this_actor::get_cname(), name.c_str(), timeFinish - timeStart);

    simgrid::s4u::this_actor::exit();
}

simgrid::s4u::ActorPtr Task::Execute(simgrid::s4u::Host* host) {
    return simgrid::s4u::Actor::create("compute", host, DoExecute, Flops, Name);
}