#include "tasks_graph.h"

#include <iostream>

XBT_LOG_NEW_DEFAULT_CATEGORY(tasks_graph, "Tasks graph log");

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
        try {
            Inputs[inputName] = ParseSize(inputDescription["size"].as<std::string>(), SizeSuffixes);
        } catch (std::exception& e) {
            XBT_ERROR("Can't parse input size: %s", e.what());
            XBT_WARN("Input size will be set to 0");
            Inputs[inputName] = 0;
        }
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

    MakeGraph();
}

void TasksGraph::MakeGraph() {
    XBT_INFO("Making graph");
    for (const auto& item: Tasks) {
        std::string taskName = item.first;
        std::shared_ptr<Task> task = item.second;
        for (const auto& input: task->GetRawInputs()) {
            std::string inputSource = input.second;
            size_t delimeterPos = inputSource.find('.');
            if (delimeterPos != std::string::npos) {
                std::string inputSourceName = inputSource.substr(0, delimeterPos);
                xbt_assert(Tasks.count(inputSourceName),
                           "Task with name \"%s\" is not presented, but its result is used as input!", 
                           inputSourceName.c_str());
                ReverseEdges[taskName].push_back(inputSourceName);
                ++OutputDegree[inputSourceName];
            }
        }
    }
    XBT_INFO("Done!");
}

void TasksGraph::MakeOrderDFS(const std::string& vertex, 
                              std::vector<std::shared_ptr<Task>>& order, 
                              std::map<std::string, bool>& used) const {
    used[vertex] = true;
    if (ReverseEdges.count(vertex)) {
        for (const std::string& neighbourVertex: ReverseEdges.at(vertex)) {
            if (!used[neighbourVertex]) {
                MakeOrderDFS(neighbourVertex, order, used);
            }
        }
    }
    order.push_back(Tasks.at(vertex));
}

std::vector<std::shared_ptr<Task>> TasksGraph::MakeTasksOrder() const {
    std::vector<std::shared_ptr<Task>> result;

    std::map<std::string, bool> used;
    for (const auto& item: Tasks) {
        std::string taskName = item.first;
        int taskOutputDegree = 0;
        if (OutputDegree.count(taskName)) {
            taskOutputDegree = OutputDegree.at(taskName);
        }
        if (!used[taskName] && taskOutputDegree == 0) {
            MakeOrderDFS(taskName, result, used);
        }
    }
    xbt_assert(result.size() == Tasks.size(), "Something went wrong, not all tasks are included in tasks order!");
    
    return result;
}

void TasksGraph::PrintGraph() const {
    std::cout << "Printing tasks..." << std::endl;
    for (const auto& item: Tasks) {
        std::cout << "Task name: " << item.second->GetName() << std::endl;
        
        for (const auto& output: item.second->GetOutputs()) {
            std::cout << "Output name: " << output.first << std::endl;
            std::cout << "Output size: " << output.second << std::endl;
        }

        for (const auto& input: item.second->GetRawInputs()) {
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
