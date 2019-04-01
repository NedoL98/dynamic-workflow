#include "task.h"

#include <exception>
#include <iostream>
#include <simgrid/s4u/VirtualMachine.hpp>

#include <map>
#include <string>
#include <vector>

using std::find;
using std::map;
using std::string;
using std::vector;


XBT_LOG_NEW_DEFAULT_CATEGORY(task, "Task log");

Task::Task(const YAML::Node& taskDescription) {
    xbt_assert(taskDescription["name"], "Task name is not specified!");
    Name = taskDescription["name"].as<string>();

    xbt_assert(taskDescription["inputs"], "Task inputs are not specified!");
    for (const YAML::Node& inputDescription: taskDescription["inputs"]) {
        xbt_assert(inputDescription["name"], "Input name is not specified!");
        string inputName = inputDescription["name"].as<string>();

        if (inputDescription["source"]) {
            AppendInput(inputDescription["source"].as<string>());
        }
    }

    xbt_assert(taskDescription["outputs"], "Task outputs are not specified!");
    for (const YAML::Node& outputDescription: taskDescription["outputs"]) {
        xbt_assert(outputDescription["name"], "Output name is not specified!");
        string outputName = outputDescription["name"].as<string>();

        xbt_assert(outputDescription["size"], "Output size is not specified!");
        string outputSize = outputDescription["size"].as<string>();

        AppendOutput(outputName, outputSize);
    }

    if (!taskDescription["requirements"]) {
        XBT_DEBUG("Task requirements are not specified, they will be set to default!");
        SetDefaultRequirements();
    } else {
        if (!taskDescription["requirements"]["cpu"]) {
            XBT_DEBUG("CPU usage is not specified for task, it will be set to default!");
            Cores = DefaultCores;
        } else {
            Cores = taskDescription["requirements"]["cpu"].as<int>();
        }

        if (!taskDescription["requirements"]["memory"]) {
            XBT_DEBUG("Memory usage is not specified for task, it will be set to default!");
            Memory = DefaultMemory;
        } else {
            try {
                Memory = ParseSize(taskDescription["requirements"]["memory"].as<string>(), SizeSuffixes);
            } catch (std::exception& e) {
                XBT_ERROR("Can't parse memory requirement: %s", e.what());
                XBT_DEBUG("Memory requirement will be set to %d", DefaultMemory);
                Memory = DefaultMemory;
            }
        }
    }

    xbt_assert(taskDescription["size"], "Task size must be specified!");
    try {
        Flops = ParseSize(taskDescription["size"].as<string>(), PerformanceSuffixes);
    } catch (std::exception& e) {
        xbt_assert("Can't parse size requirement: %s", e.what());
    }
}

const string& Task::GetName() const {
    return Name;
}

const vector<string>& Task::GetInputs() const {
    return Inputs;
}

const map<string, double>& Task::GetOutputs() const {
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

void Task::AppendRawInput(const string& name, const string& source) {
    if (RawInputs.count(name) > 0) {
        XBT_WARN("Input name is not unique! Previous input will be deleted!");
    }
    RawInputs[name] = source;
}

void Task::AppendInput(const string& name) {
    if (find(Inputs.begin(), Inputs.end(), name) == Inputs.end()) {
        Inputs.push_back(name);
    }
}

void Task::AppendOutput(const string& name, const string& size) {
    if (Outputs.count(name) > 0) {
        XBT_WARN("Output name is not unique! Previous output will be deleted!");
    }
    
    try {
        Outputs[name] = ParseSize(size, SizeSuffixes);
    } catch (std::exception& e) {
        XBT_ERROR("Can't parse output size: %s", e.what());
        XBT_DEBUG("Output size will be set to default value");
        Outputs[name] = 0;
    }
}

void Task::DoExecute(double flops, 
                     string name,
                     const std::function<void(int, void*)>& actorFinishFunction,
                     void* context) {
    simgrid::s4u::this_actor::on_exit(actorFinishFunction, context);

    simgrid::s4u::Host* host = simgrid::s4u::this_actor::get_host();

    double timeStart = simgrid::s4u::Engine::get_clock();
    host->execute(flops);
    double timeFinish = simgrid::s4u::Engine::get_clock();

    XBT_INFO("%s: %s task %s executed %g", host->get_cname(),
            simgrid::s4u::this_actor::get_cname(), name.c_str(), timeFinish - timeStart);

    simgrid::s4u::this_actor::exit();
}

simgrid::s4u::ActorPtr Task::Execute(simgrid::s4u::VirtualMachine* vm,
                                     const std::function<void(int, void*)>& actorFinishFunction,
                                     void* context) {
    simgrid::s4u::ActorPtr actor = simgrid::s4u::Actor::create("compute", vm, DoExecute, Flops, Name, actorFinishFunction, context);
    actor->set_property("size", std::to_string(Flops));
    return actor;
}

bool Task::CanExecute(const simgrid::s4u::Host* host) const {
    int availableCores = std::stoi(host->get_property("cores"));
    double availableMemory = std::stod(host->get_property("memory"));

    return availableCores >= Cores && availableMemory >= Memory;
}

bool Task::CanExecute(const VMDescription& vmDescr) const {
    return vmDescr.GetCores() >= Cores && vmDescr.GetMemory() >= Memory;
}

simgrid::s4u::VirtualMachine* Task::MakeVirtualMachine(simgrid::s4u::Host* host) {
    simgrid::s4u::VirtualMachine* vm = new simgrid::s4u::VirtualMachine(Name + "_VM", host, Cores, Memory);
    host->set_property("cores", std::to_string(std::stoi(host->get_property("cores")) - Cores));
    host->set_property("memory", std::to_string(std::stod(host->get_property("memory")) - Memory));
    Host = host;
    return vm;
}

void Task::ConsolidateTask(const Task& other) {
    Cores = std::max(Cores, other.Cores);
    Memory = std::max(Memory, other.Memory);
    Flops += other.Flops;

    for (const auto& elem: other.RawInputs) {
        if (elem.second.find('.') == string::npos && RawInputs.count(elem.first) == 0) {
            RawInputs[elem.first] = elem.second;
        }
    }

    Outputs = other.Outputs;
}

void Task::Finish(simgrid::s4u::ActorPtr vmPtr) {
    if (Done) {
        return;
    }
    vmPtr->join();
    if (Host) {
        Host->set_property("cores", std::to_string(std::stoi(Host->get_property("cores")) + Cores));
        Host->set_property("memory", std::to_string(std::stod(Host->get_property("memory")) + Memory));
        Host = nullptr;
    }
    Done = true;
}
