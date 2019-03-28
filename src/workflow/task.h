#pragma once
#include <string>
#include <vector>
#include <map>
#include <workflow/file.h>
#include <yaml-cpp/yaml.h>
#include <spec.h>

namespace Workflow {
    struct Task {
        std::string Name;
        std::vector<int> Inputs, Outputs;
        std::vector<int> Dependencies, Successors;
        TaskSpec Requirements;

        EState state;
        Task(const YAML::Node& taskDescription, const FileRegistry& registry);
    };
}
