#include "task.h"

#include <exception>
#include <iostream>
#include <simgrid/s4u/VirtualMachine.hpp>

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

    if (!taskDescription["requirements"]) {
        XBT_WARN("Task requirements are not specified, they will be set to default!");
        SetDefaultRequirements();
    } else {
        if (!taskDescription["requirements"]["cpu"]) {
            XBT_WARN("CPU usage is not specified for task, it will be set to default!");
            Cores = DefaultCores;
        } else {
            Cores = taskDescription["requirements"]["cpu"].as<int>();
        }

        if (!taskDescription["requirements"]["memory"]) {
            XBT_WARN("Memory usage is not specified for task, it will be set to default!");
            Memory = DefaultMemory;
        } else {
            try {
                Memory = ParseNumber(taskDescription["requirements"]["memory"].as<std::string>(), SizeSuffixes);
            } catch (std::exception& e) {
                XBT_ERROR("Can't parse memory requirement: %s", e.what());
                XBT_WARN("Memory requirement will be set to %d", DefaultMemory);
                Memory = DefaultMemory;
            }
        }
    }

    xbt_assert(taskDescription["size"], "Task size must be specified!");
    try {
        Flops = ParseNumber(taskDescription["size"].as<std::string>(), PerformanceSuffixes);
    } catch (std::exception& e) {
        xbt_assert("Can't parse size requirement: %s", e.what());
    }
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
    return Memory;
}

double Task::GetSize() const {
    return Flops;
}

void Task::SetDefaultRequirements() {
    Cores = DefaultCores;
    Memory = DefaultMemory;
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
    
    try {
        Outputs[name] = ParseNumber(size, SizeSuffixes);
    } catch (std::exception& e) {
        XBT_ERROR("Can't parse output size: %s", e.what());
        XBT_WARN("Output size will be set to default value");
        Outputs[name] = 0;
    }
}

void Task::DoExecute(double flops, std::string name) {
    simgrid::s4u::Host* host = simgrid::s4u::this_actor::get_host();

    double timeStart = simgrid::s4u::Engine::get_clock();
    host->execute(flops);
    double timeFinish = simgrid::s4u::Engine::get_clock();

    XBT_INFO("%s: %s task %s executed %g", host->get_cname(),
            simgrid::s4u::this_actor::get_cname(), name.c_str(), timeFinish - timeStart);

    simgrid::s4u::this_actor::exit();
}

simgrid::s4u::ActorPtr Task::Execute(simgrid::s4u::VirtualMachine* vm) {
    simgrid::s4u::ActorPtr actor = simgrid::s4u::Actor::create("compute", vm, DoExecute, Flops, Name);
    actor->set_property("size", std::to_string(Flops));
    return actor;
}

bool Task::CanExecute(simgrid::s4u::Host* host) {
    int availableCores = std::stoi(host->get_property("cores"));
    double availableMemory = std::stod(host->get_property("memory"));

    return availableCores >= Cores && availableMemory >= Memory;
}

simgrid::s4u::VirtualMachine* Task::MakeVirtualMachine(simgrid::s4u::Host* host) {
    simgrid::s4u::VirtualMachine* vm = new simgrid::s4u::VirtualMachine(Name + "_VM", host, Cores, Memory);
    host->set_property("cores", std::to_string(std::stoi(host->get_property("cores")) - Cores));
    host->set_property("memory", std::to_string(std::stod(host->get_property("memory")) - Memory));
    Host = host;
    return vm;
}

void Task::Finish(simgrid::s4u::ActorPtr vmPtr) {
    if (Host == nullptr) {
        if (!Done) {
            XBT_WARN("Trying to finish task %s, but it is not computing on any host!", Name.c_str());
        }
        return;
    }
    vmPtr->join();
    Host->set_property("cores", std::to_string(std::stoi(Host->get_property("cores")) + Cores));
    Host->set_property("memory", std::to_string(std::stod(Host->get_property("memory")) + Memory));
    Host = nullptr;
    Done = true;
}