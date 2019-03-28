#pragma once
#include <string>
#include <vector>
#include <map>
#include <workflow/file.h>
#include <yaml-cpp/yaml.h>
#include <spec.h>

struct Node {
    typedef std::map<std::string, FileDescription> FileRegistry;
    std::string Name;
    std::vector<int> Inputs;
    std::vector<int> Outputs;
    TaskSpec Requirements;

    EState state;
    Node(const YAML::Node& taskDescription, const FileRegistry& registry);
};
