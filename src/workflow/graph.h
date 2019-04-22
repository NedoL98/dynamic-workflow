#pragma once
#include <vector>
#include <memory>
#include <string>
#include <set>

#include "argument_parser.h"
#include <workflow/task.h>
#include <files/manager.h>

namespace Workflow {
    class Graph {
        void BuildDependencies();
        std::set<int> FinishedTasks;
    public:
        std::vector<std::unique_ptr<Task> > Nodes;
        std::map<std::string, int> TaskName2Id;
        std::vector<int> Inputs;
        std::vector<int> Outputs;
        std::string Name;
        Manager FileManager;
        double Deadline;

        Graph(const std::string& filename, cxxopts::ParseResult& options);

        void MakeOrderDFS(int v, std::vector<Task>& order, std::vector<bool>& used) const;
        std::vector<Task> MakeTasksOrder() const;

        bool IsFinished() const;
        void FinishTask(int id); 
        void AssignTask(int TaskId, int hostId); 
    };
}
