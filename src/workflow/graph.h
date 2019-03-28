#pragma once
#include <vector>
#include <memory>
#include <string>

#include <workflow/node.h>
#include <workflow/file.h>

class WorkflowGraph {
public:
    std::vector<std::unique_ptr<Node> > Nodes;
    std::map<std::string, FileDescription> Files;
    std::map<int, FileDescription> FileIdMapping;
    std::vector<int> Inputs;
    std::vector<int> Outputs;
    std::string Name;
    double Deadline;
    WorkflowGraph(const std::string& filename);
};
