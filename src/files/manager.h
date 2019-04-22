#pragma once
#include <vector>
#include <set>
#include <map>
#include <yaml-cpp/yaml.h>
#include <files/description.h>

namespace Workflow {
    class Graph;
}

class Manager {
    std::map<int, std::set<int>> TaskDerivatives;
    std::map<int, std::set<int>> TaskDependencies;
    std::map<int, int> TaskToHost;
    std::map<std::string, FileDescription> Files;
    std::map<int, FileDescription> FileIdMapping;
    std::set<int> WorkflowOutputs, WorkflowInputs;
    int OutputProduced;
    Workflow::Graph *Owner;

public:
    Manager(Workflow::Graph *owner):
        OutputProduced(0), 
        Owner(owner) {
    }

    int TryAddFile(const YAML::Node& description);

    const FileDescription& GetFileById(int id) const;

    const FileDescription& GetFileByName(const std::string& s) const;

    void RegisterTaskFiles(int taskId);

    void SetInputs(const std::vector<int>& data);
    void SetOutputs(const std::vector<int>& data);
    void SetAuthor(int fileId, int taskId);
    void SetReceiver(int fileId, int taskId);

    void FinishTask(int taskId);

    void AssignTask(int taskId, int hostId);

    bool IsFinished() const;

    void BeginTransfer(const FileDescription& file);
};
