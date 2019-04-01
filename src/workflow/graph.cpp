#include <common/common.h>
#include <workflow/graph.h>
#include <simgrid/s4u.hpp>
#include <memory>

#include "graph.h"

using std::vector;
using std::map;
using std::string;

XBT_LOG_NEW_DEFAULT_CATEGORY(workflow_graph_cpp, "Tasks graph log");

namespace Workflow {
    Graph::Graph(const string& filename, cxxopts::ParseResult& options) {
        XBT_INFO("Loading workflow graph from %s", filename.c_str());
        YAML::Node tasksGraph = YAML::LoadFile(filename);
        xbt_assert(tasksGraph["name"], "Workflow name is not specified!");
        Name = tasksGraph["name"].as<string>();
        if (options.count("deadline")) {
            Deadline = options["deadline"].as<double>();
        } else if (tasksGraph["deadline"]) {
            Deadline = tasksGraph["deadline"].as<double>();
        } else {
            xbt_die("Task deadline is not specified in workflow file nor as a command line argument!");    
        }
        xbt_assert(tasksGraph["inputs"], "Tasks inputs are not specified!");
        for (const YAML::Node& inputDescription: tasksGraph["inputs"]) {
            xbt_assert(inputDescription["name"], "Input name is not specified!");
            string inputName = inputDescription["name"].as<string>(); 
            if (Files.count(inputName) > 0) {
                XBT_WARN("Input name is not unique! Previous input will be deleted!");
            }
            xbt_assert(inputDescription["size"], "Input size is not specified!");
            Workflow::FileRegistry::iterator insertionResult;
            try {
                insertionResult = Files.insert({inputName, FileDescription(inputName, ParseSize(inputDescription["size"].as<string>(), SizeSuffixes))}).first;
            } catch (std::exception& e) {
                XBT_ERROR("Can't parse input size: %s", e.what());
                XBT_WARN("Input size will be set to 0");
                insertionResult = Files.insert({inputName, FileDescription(inputName, 0)}).first;
            }
            FileIdMapping.insert({insertionResult->second.Id, insertionResult->second});
            Inputs.push_back(insertionResult->second.Id);
        }

        xbt_assert(tasksGraph["outputs"], "Tasks outputs are not specified!");
        for (const YAML::Node& outputDescription: tasksGraph["outputs"]) {
            xbt_assert(outputDescription["name"], "Output name is not specified!");
            string outputName = outputDescription["name"].as<string>(); 
            xbt_assert(outputDescription["source"], "Output source is not specified!");
            if (Files.count(outputName) > 0) {
                XBT_WARN("Output name is not unique! Previous output will be deleted!");
            }
            auto fileIterator = Files.insert({outputName, FileDescription(outputName, 0)}).first;
            FileIdMapping.insert({fileIterator->second.Id, fileIterator->second});
            Outputs.push_back(fileIterator->second.Id);
        }

        xbt_assert(tasksGraph["tasks"], "No tasks are specified in input file!");
        for (const YAML::Node& taskDescription: tasksGraph["tasks"]) {
            xbt_assert(taskDescription["name"], "Task name is not specified!");
            TaskName2Id[taskDescription["name"].as<string>()] = Nodes.size();

            for (const YAML::Node& inputDescription: taskDescription["inputs"]) {
                xbt_assert(inputDescription["name"], "Input name is not specified!");
                string inputName = inputDescription["name"].as<string>();
                if (!Files.count(inputName)) {
                    auto fileIterator = Files.insert({inputName, FileDescription(inputName, 0)}).first;
                    FileIdMapping.insert({fileIterator->second.Id, fileIterator->second});
                }
            }
            for (const YAML::Node& outputDescription: taskDescription["outputs"]) {
                xbt_assert(outputDescription["name"], "Output name is not specified!");
                string outputName = outputDescription["name"].as<string>();

                xbt_assert(outputDescription["size"], "Output size is not specified!");
                long long outputSize = ParseSize(outputDescription["size"].as<string>(), SizeSuffixes);
                
                auto fileIterator = Files.find(outputName);
                if (fileIterator == Files.end()) {
                    fileIterator = Files.insert({outputName, FileDescription(outputName, outputSize)}).first;
                    FileIdMapping.insert({fileIterator->second.Id, fileIterator->second});
                } else {
                    fileIterator->second.Size = outputSize;
                }
            }
            Nodes.push_back(std::make_unique<Task>(taskDescription, Files, Nodes.size()));
        }

        XBT_INFO("Done!");
        BuildDependencies();
    }

    void Graph::BuildDependencies() {
        map<int, int> FileId2Owner;
        for (int nodeId = 0; nodeId < Nodes.size(); nodeId++) {
            for (int fileId : Nodes[nodeId]->Outputs) {
                FileId2Owner[fileId] = nodeId;
            }
        }
        for (int nodeId = 0; nodeId < Nodes.size(); nodeId++) {
            for (int fileId : Nodes[nodeId]->Inputs) {
                auto fileOwner = FileId2Owner.find(fileId);
                if (fileOwner != FileId2Owner.end()) {
                    Nodes[nodeId]->Dependencies.push_back(fileOwner->second);
                    Nodes[fileOwner->second]->Successors.push_back(nodeId);
                }
            }
        }
    }

    void Graph::MakeOrderDFS(int v, 
                             vector<Task>& order, 
                             vector<bool>& used) const {
        used[v] = true;
        for (int u: Nodes[v]->GetSuccessors()) {
            if (!used[u]) {
                MakeOrderDFS(u, order, used);
            }
        }
        order.push_back(*Nodes[v]);
    }

    vector<Task> Graph::MakeTasksOrder() const {
        vector<Task> order;

        vector<bool> used(Nodes.size());
        for (int i = 0; i < Nodes.size(); ++i) {
            if (!used[i] && Nodes[i]->GetDependencies().size() == 0) {
                MakeOrderDFS(i, order, used);
            }
        }
        xbt_assert(order.size() == Nodes.size(), "Something went wrong, not all tasks are included in tasks order!");
        
        std::reverse(order.begin(), order.end());

        return order;
    }

}
