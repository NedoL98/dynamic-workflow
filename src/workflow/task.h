#pragma once
#include <string>
#include <vector>
#include <map>
#include <workflow/file.h>
#include <yaml-cpp/yaml.h>
#include <spec.h>

namespace Workflow {
    class Graph;
    class Task {
        std::string Name;
        std::vector<int> Inputs, Outputs;
        std::vector<int> Dependencies, Successors;
        TaskSpec Requirements;

        EState state;
        int Id;
    public:
        Task(const YAML::Node& taskDescription, const FileRegistry& registry, int Id);

        int GetId() const {
            return Id;
        }
        const std::vector<int>& GetDependencies() const {
            return Dependencies;
        }
        const std::vector<int>& GetSuccessors() const {
            return Successors;
        }
        const TaskSpec& GetTaskSpec() const {
            return Requirements;
        }
        
        const std::string& GetName() const {
            return Name;
        }

        bool CanBeExecuted(const VMDescription& vmDescr) const {
            return Requirements.Cores <= vmDescr.GetCores() && Requirements.Memory <= vmDescr.GetMemory();
        }

        friend class Workflow::Graph;
    };
}
