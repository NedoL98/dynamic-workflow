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
    Graph::Graph(const string& filename, cxxopts::ParseResult& options):
        FileManager(this) {
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
            Inputs.push_back(FileManager.TryAddFile(inputDescription));
        }

        xbt_assert(tasksGraph["tasks"], "No tasks are specified in input file!");
        for (const YAML::Node& taskDescription: tasksGraph["tasks"]) {
            for (const YAML::Node& outputDescription: taskDescription["outputs"]) {
                FileManager.TryAddFile(outputDescription);
            }
        }
        for (const YAML::Node& taskDescription: tasksGraph["tasks"]) {
            xbt_assert(taskDescription["name"], "Task name is not specified!");
            TaskName2Id[taskDescription["name"].as<string>()] = Nodes.size();
            Nodes.push_back(std::make_unique<Task>(taskDescription, FileManager, Nodes.size()));
        }
        xbt_assert(tasksGraph["outputs"], "Tasks outputs are not specified!");
        for (const YAML::Node& description: tasksGraph["outputs"]) {
            Outputs.push_back(FileManager.GetFileByName(description["name"].as<string>()).Id);
        }
        FileManager.SetInputs(Inputs);
        FileManager.SetOutputs(Outputs);
        XBT_INFO("Done!");
        BuildDependencies();
    }

    void Graph::BuildDependencies() {
        map<int, int> FileId2Owner;
        for (size_t nodeId = 0; nodeId < Nodes.size(); nodeId++) {
            for (int fileId : Nodes[nodeId]->GetOutputs()) {
                FileId2Owner[fileId] = nodeId;
            }
        }
        for (size_t nodeId = 0; nodeId < Nodes.size(); nodeId++) {
            for (int fileId : Nodes[nodeId]->GetInputs()) {
                FileManager.SetReceiver(fileId, nodeId);
                auto fileOwner = FileId2Owner.find(fileId);
                if (fileOwner != FileId2Owner.end()) {
                    Nodes[nodeId]->AddDependency(fileOwner->second);
                    Nodes[fileOwner->second]->AddSuccessor(nodeId);
                    FileManager.SetAuthor(fileId, fileOwner->second);
                } else {
                    Nodes[nodeId]->IncFinishedTransfers();
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
        for (size_t i = 0; i < Nodes.size(); ++i) {
            if (!used[i] && Nodes[i]->GetDependencies().size() == 0) {
                MakeOrderDFS(i, order, used);
            }
        }
        xbt_assert(order.size() == Nodes.size(), "Something went wrong, not all tasks are included in tasks order!");
        std::reverse(order.begin(), order.end());
        return order;
    }
    Task& Graph::GetTask(int id) {
        return *Nodes[id];
    }

    const Task& Graph::GetTask(int id) const {
        return *Nodes[id];
    }

    Task& Graph::GetTaskByName(const std::string& name) {
        return *Nodes[TaskName2Id.find(name)->second];
    }

    const Task& Graph::GetTaskByName(const std::string& name) const {
        return *Nodes[TaskName2Id.find(name)->second];
    }

    size_t Graph::GetTaskNumber() const {
        return Nodes.size();
    }

    double Graph::GetDeadline() const {
        return Deadline;
    }

    AbstractGraph::GraphAbstractIterator* Graph::GetGraphIterator() {
        return new Graph::GraphIterator(Nodes);
    }


    FileDescription::FileIterator Graph::GetReadyFilesIterator() const {
        return FileManager.GetReadyFilesIterator();
    }

    bool Graph::IsFinished() const {
        return FinishedTasks.size() == Nodes.size() && FileManager.IsFinished();
    }

    void Graph::FinishTask(int id) {
        FileManager.FinishTask(id);
        if (Nodes[id]->GetState() == EState::DONE) {
            return;
        }
        Nodes[id]->SetState(EState::DONE);
        FinishedTasks.insert(id);
        for (int suc : Nodes[id]->GetSuccessors()) {
            Nodes[suc]->IncFinishedDeps();
            if (Nodes[suc]->IsReady()) {
                Nodes[suc]->SetState(EState::READY);
            }
        }
    }
    
    void Graph::StartTransfer(int fileId) {
        FileManager.StartTransfer(FileManager.GetFileById(fileId));
    }

    void Graph::FinishTransfer(int fileId) {
        auto description = FileManager.GetFileById(fileId);
        FileManager.FinishTransfer(description);
        Nodes[description.Receiver]->IncFinishedTransfers();
    }

    void Graph::AssignTask(int taskId, int hostId) {
        FileManager.AssignTask(taskId, hostId);
    }
}
