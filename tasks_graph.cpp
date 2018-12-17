#include "tasks_graph.h"

#include <iostream>

XBT_LOG_NEW_DEFAULT_CATEGORY(tasks_graph, "Logging tasks_graph");

TasksGraph::TasksGraph(std::string filepath) {
    XBT_INFO("Loading tasks graph from %s", filepath.c_str());
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
        std::shared_ptr<Task> task(new Task(taskDescription));

        if (Tasks.count(task->GetName()) > 0) {
            XBT_WARN("Task name is not unique! Previous task will be deleted!");
        }

        Tasks[task->GetName()] = task;
    }

    XBT_INFO("Done!");
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