#pragma once
#include <vector>
#include <memory>
#include <string>
#include <set>

#include <prototypes/simulator.h>
#include "argument_parser.h"
#include <workflow/task.h>
#include <workflow/abstract_graph.h>
#include <files/manager.h>

namespace Workflow {
    class Graph : public AbstractGraph {
        void BuildDependencies();
        std::set<int> FinishedTasks;
        std::vector<std::unique_ptr<Task> > Nodes;
        std::map<std::string, int> TaskName2Id;
        std::vector<int> Inputs;
        std::vector<int> Outputs;
        double Deadline;
        Manager FileManager;
    public:
        class GraphIterator : public GraphAbstractIterator {
            std::vector<std::unique_ptr<Task>>& data;
            int i = 0;
        public:
            GraphIterator(std::vector<std::unique_ptr<Task>>& v, int pos = 0);
            virtual Task& Get() override;
            virtual const Task& Get() const override;
            virtual void Next() override;
            virtual operator bool() const override;
        };


        Graph(const std::string& filename, cxxopts::ParseResult& options);

        void MakeOrderDFS(int v, std::vector<Task>& order, std::vector<bool>& used) const;
        std::vector<Task> MakeTasksOrder() const;

        virtual Task& GetTask(int id) override; 
        virtual const Task& GetTask(int id) const override;
        virtual Task& GetTaskByName(const std::string& name) override; 
        virtual const Task& GetTaskByName(const std::string& name) const override;
        virtual size_t GetTaskNumber() const override;
        virtual GraphAbstractIterator* GetGraphIterator() override; 
        virtual FileDescription::FileIterator GetReadyFilesIterator() const override;

        double GetDeadline() const;

        virtual bool IsFinished() const override;
        virtual void FinishTask(int id) override; 
        virtual void StartTransfer(const FileDescription& d) override;
        virtual void FinishTransfer(const FileDescription& d) override; 
        virtual void AssignTask(int TaskId, int hostId) override;
    };
}
