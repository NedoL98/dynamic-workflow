#include "tasks_graph.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

XBT_LOG_NEW_DEFAULT_CATEGORY(tasks_graph, "Tasks graph log");

TasksGraph::TasksGraph(string filepath) {
    XBT_INFO("Loading tasks graph from %s", filepath.c_str());
    YAML::Node tasksGraph = YAML::LoadFile(filepath);

    xbt_assert(tasksGraph["name"], "Tasks graph name is not specified!");
    Name = tasksGraph["name"].as<string>();

    xbt_assert(tasksGraph["inputs"], "Tasks inputs are not specified!");
    for (const YAML::Node& inputDescription: tasksGraph["inputs"]) {
        xbt_assert(inputDescription["name"], "Input name is not specified!");
        string inputName = inputDescription["name"].as<string>(); 
        if (Inputs.count(inputName) > 0) {
            XBT_WARN("Input name is not unique! Previous input will be deleted!");
        }
        xbt_assert(inputDescription["size"], "Input size is not specified!");
        try {
            Inputs[inputName] = ParseSize(inputDescription["size"].as<string>(), SizeSuffixes);
        } catch (std::exception& e) {
            XBT_ERROR("Can't parse input size: %s", e.what());
            XBT_WARN("Input size will be set to 0");
            Inputs[inputName] = 0;
        }
    }

    xbt_assert(tasksGraph["outputs"], "Tasks outputs are not specified!");
    for (const YAML::Node& outputDescription: tasksGraph["outputs"]) {
        xbt_assert(outputDescription["name"], "Output name is not specified!");
        string outputName = outputDescription["name"].as<string>(); 
        xbt_assert(outputDescription["source"], "Output source is not specified!");
        if (Outputs.count(outputName) > 0) {
            XBT_WARN("Output name is not unique! Previous output will be deleted!");
        }
        Outputs[outputName] = outputDescription["source"].as<string>();
    }

    xbt_assert(tasksGraph["tasks"], "No tasks are specified in input file!");
    for (const YAML::Node& taskDescription: tasksGraph["tasks"]) {
        shared_ptr<Task> task(new Task(taskDescription));

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
        string taskName = item.first;
        shared_ptr<Task> task = item.second;
        for (const auto& input: task->GetRawInputs()) {
            string inputSource = input.second;
            size_t delimeterPos = inputSource.find('.');
            if (delimeterPos != string::npos) {
                string inputSourceName = inputSource.substr(0, delimeterPos);
                xbt_assert(Tasks.count(inputSourceName),
                           "Task with name \"%s\" is not presented, but its result is used as input!", 
                           inputSourceName.c_str());
                ReverseEdges[taskName].push_back(inputSourceName);
                ++OutputDegree[inputSourceName];
            }
        }
        if (!OutputDegree.count(taskName)) {
            OutputDegree[taskName] = 0;
        }
    }
    XBT_INFO("Done!");
}

void TasksGraph::MakeOrderDFS(const string& vertex, 
                              vector<shared_ptr<Task>>& order, 
                              map<string, bool>& used, 
                              bool reverse) const {
    if (!reverse) {
        order.push_back(Tasks.at(vertex));
    }
    used[vertex] = true;
    if (ReverseEdges.count(vertex)) {
        for (const string& neighbourVertex: ReverseEdges.at(vertex)) {
            if (!used[neighbourVertex]) {
                MakeOrderDFS(neighbourVertex, order, used, reverse);
            }
        }
    }
    if (reverse) {
        order.push_back(Tasks.at(vertex));
    }
}

vector<shared_ptr<Task>> TasksGraph::MakeTasksOrder(bool reverse) const {
    vector<shared_ptr<Task>> result;

    map<string, bool> used;
    for (const auto& item: Tasks) {
        string taskName = item.first;
        if (!used[taskName] && OutputDegree.at(taskName) == 0) {
            MakeOrderDFS(taskName, result, used, reverse);
        }
    }
    xbt_assert(result.size() == Tasks.size(), "Something went wrong, not all tasks are included in tasks order!");
    
    return result;
}

int TasksGraph::Size() const {
    return Tasks.size();
}

int TasksGraph::MaxCores() const {
    return std::max_element(Tasks.begin(), Tasks.end(), 
        [] (const auto& lhs, const auto& rhs) {
            return lhs.second->GetCores() < rhs.second->GetCores();
        })->second->GetCores();
}

double TasksGraph::MaxMemory() const {
    return std::max_element(Tasks.begin(), Tasks.end(), 
        [] (const auto& lhs, const auto& rhs) {
            return lhs.second->GetMemory() < rhs.second->GetMemory();
        })->second->GetMemory();
}

std::map<std::string, VMDescription> TasksGraph::GetCheapestVMs(const VMList& vmList) const {
    std::map<std::string, VMDescription> cheapestVM;
    for (const auto& elem: Tasks) {
        int bestPrice = -1;
        for (VMDescription vmDescr: vmList) {
            if (elem.second->CanExecute(vmDescr) && (bestPrice == -1 || vmDescr.GetPrice() < bestPrice)) {
                bestPrice = vmDescr.GetPrice();
                cheapestVM.insert({elem.first, vmDescr});
            }
        }
    }
    return cheapestVM;
}

void TasksGraph::PrintGraph() const {

    using namespace std;

    cout << "Printing tasks..." << endl;
    for (const auto& item: Tasks) {
        cout << "Task name: " << item.second->GetName() << endl;
        
        for (const auto& output: item.second->GetOutputs()) {
            cout << "Output name: " << output.first << endl;
            cout << "Output size: " << output.second << endl;
        }

        for (const auto& input: item.second->GetRawInputs()) {
            cout << "Input name: " << input.first << endl;
            cout << "Input source: " << input.second << endl; 
        }
    }

    cout << "Printing inputs..." << endl;
    for (const auto& item: Inputs) {
        cout << "Input name: " << item.first << endl;
        cout << "Input size: " << item.second << endl;
    }

    cout << "Printing outputs..." << endl;
    for (const auto& item: Outputs) {
        cout << "Output name: " << item.first << endl;
        cout << "OUtput source: " << item.second << endl;
    }
}
