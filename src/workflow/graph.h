#pragma once
#include <vector>
#include <memory>
#include <string>

#include <workflow/task.h>
#include <workflow/file.h>

namespace Workflow {
    class Graph {
    public:
        std::vector<std::unique_ptr<Task> > Nodes;
        std::map<std::string, FileDescription> Files;
        std::map<int, FileDescription> FileIdMapping;
        std::vector<int> Inputs;
        std::vector<int> Outputs;
        std::string Name;
        double Deadline;
        Graph(const std::string& filename);
    };
}
