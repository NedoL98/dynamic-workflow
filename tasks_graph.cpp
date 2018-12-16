#include "tasks_graph.h"

#include <iostream>
#include <yaml-cpp/yaml.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(tasks_graph, "Logging tasks_graph");

TasksGraph::TasksGraph(std::string filepath) {
    YAML::Node tasksGraph = YAML::LoadFile(filepath);

    xbt_assert(tasksGraph["name"], "Tasks graph name is not specified!");
    Name = tasksGraph["name"].as<std::string>();

    xbt_assert(tasksGraph["inputs"], "Tasks inputs are not specified!");
    for (const YAML::Node& inputDescription: tasksGraph["inputs"]) {
        xbt_assert(inputDescription["name"], "Input name is not specified!");
        std::string inputName = inputDescription["name"].as<std::string>(); 
        if (Inputs.count(inputName) > 0) {
            XBT_WARN("Input name is not unique! Previous input will be deleted!");
        }
        xbt_assert(inputDescription["size"], "Input size is not specified!");
        Inputs[inputName] = ParseNumber(inputDescription["size"].as<std::string>(), SizeSuffixes);
    }

    xbt_assert(tasksGraph["outputs"], "Tasks outputs are not specified!");
    for (const YAML::Node& outputDescription: tasksGraph["outputs"]) {
        xbt_assert(outputDescription["name"], "Output name is not specified!");
        std::string outputName = outputDescription["name"].as<std::string>(); 
        xbt_assert(outputDescription["source"], "Output source is not specified!");
        if (Outputs.count(outputName) > 0) {
            XBT_WARN("Output name is not unique! Previous output will be deleted!");
        }
        Outputs[outputName] = outputDescription["source"].as<std::string>();
    }
    
    xbt_assert(tasksGraph["tasks"], "No tasks are specified in input file!");
    for (const YAML::Node& taskDescription: tasksGraph["tasks"]) {
        std::shared_ptr<Task> task(new Task);

        xbt_assert(taskDescription["name"], "Task name is not specified!");
        task->SetName(taskDescription["name"].as<std::string>());

        xbt_assert(taskDescription["inputs"], "Task inputs are not specified!");
        for (const YAML::Node& inputDescription: taskDescription["inputs"]) {
            xbt_assert(inputDescription["name"], "Input name is not specified!");
            std::string inputName = inputDescription["name"].as<std::string>();

            xbt_assert(inputDescription["source"], "Source for input is not specified!");
            std::string sourceName = inputDescription["source"].as<std::string>();

            task->AppendInput(inputName, sourceName);
        }

        xbt_assert(taskDescription["outputs"], "Task outputs are not specified!");
        for (const YAML::Node& outputDescription: taskDescription["outputs"]) {
            xbt_assert(outputDescription["name"], "Output name is not specified!");
            std::string outputName = outputDescription["name"].as<std::string>();

            xbt_assert(outputDescription["size"], "Output size is not specified!");
            std::string outputSize = outputDescription["size"].as<std::string>();

            task->AppendOutput(outputName, outputSize);
        }

        xbt_assert(taskDescription["requirements"], "Task requirements are not specified!");
        
        xbt_assert(taskDescription["requirements"]["cpu"], "CPU usage is not specified for task!");
        std::string cores = taskDescription["requirements"]["cpu"].as<std::string>();
        task->SetCores(cores);
        
        xbt_assert(taskDescription["requirements"]["memory"], "Memory usage is not specified for task!");
        std::string memory = taskDescription["requirements"]["memory"].as<std::string>();
        task->SetMemory(memory);

        xbt_assert(taskDescription["size"], "Task size is not specified!");
        std::string size = taskDescription["size"].as<std::string>();
        task->SetSize(size);

        if (Tasks.count(task->GetName()) > 0) {
            XBT_WARN("Task name is not unique! Previous task will be deleted!");
        }
        Tasks[task->GetName()] = task;
    }
}

void TasksGraph::PrintGraph() {
    std::cout << "Printing tasks..." << std::endl;
    for (const auto& item: Tasks) {
        std::cout << "Task name: " << item.second->GetName() << std::endl;
        
        for (const auto& output: item.second->GetOutputs()) {
            std::cout << "Output name: " << output.first << std::endl;
            std::cout << "Output size: " << output.second << std::endl;
        }

        for (const auto& input: item.second->GetInputs()) {
            std::cout << "Input name: " << input.first << std::endl;
            std::cout << "Input source: " << input.second << std::endl; 
        }
    }

    std::cout << "Printing inputs..." << std::endl;
    for (const auto& item: Inputs) {
        std::cout << "Input name: " << item.first << std::endl;
        std::cout << "Input size: " << item.second << std::endl;
    }

    std::cout << "Printing outputs..." << std::endl;
    for (const auto& item: Outputs) {
        std::cout << "Output name: " << item.first << std::endl;
        std::cout << "OUtput source: " << item.second << std::endl;
    }
}