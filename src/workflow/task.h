#pragma once
#include <string>
#include <vector>
#include <map>
#include <yaml-cpp/yaml.h>
#include <spec.h>
#include <files/manager.h>
#include <workflow/abstract_graph.h>

namespace Workflow {
    class Task {
    protected:
        std::string Name;
        std::vector<int> Inputs, Outputs;
        std::vector<int> Dependencies, Successors;
        int FinishedDeps = 0;
        int FinishedTransfers = 0;
        TaskSpec Requirements;

        EState State;
        int Id;
    public:
        Task(const YAML::Node& taskDescription, const Manager& registry, int Id);

        int GetId() const {
            return Id;
        }

        void AddDependency(int d) {
            Dependencies.push_back(d);
        }

        void AddSuccessor(int d) {
            Successors.push_back(d);
        }
        const std::vector<int>& GetDependencies() const {
            return Dependencies;
        }
        const std::vector<int>& GetSuccessors() const {
            return Successors;
        }

        const std::vector<int>& GetInputs() const {
            return Inputs;
        }
        const std::vector<int>& GetOutputs() const {
            return Outputs;
        }

        const TaskSpec& GetTaskSpec() const {
            return Requirements;
        }
        
        const std::string& GetName() const {
            return Name;
        }

        EState GetState() const {
            return State;
        }

        void SetState(EState s) {
            State = s;
        }

        int GetFinishedDeps() const {
            return FinishedDeps;
        }

        void IncFinishedDeps() {
            FinishedDeps++;
        }

        int GetFinishedTransfers() const {
            return FinishedTransfers;
        }

        void IncFinishedTransfers() {
            FinishedTransfers++;
        }
        
        bool IsReady() const {
            if (State == EState::RUNNING || State == EState::DONE) {
                return false;
            }
            return FinishedDeps == static_cast<int>(Dependencies.size());// &&
                   //FinishedTransfers == static_cast<int>(Inputs.size());
        }

        bool StartExecuting() {
            if (!IsReady()) {
                return false;
            }
            State = EState::RUNNING;
            return true;
        }
            

        bool CanBeExecuted(const VMDescription& vmDescr) const {
            return Requirements.GetCores() <= vmDescr.GetCores() && Requirements.GetMemory() <= vmDescr.GetMemory();
        }

        double GetExecutionTime(const VMDescription& vmDescr) const {
            xbt_assert(CanBeExecuted(vmDescr));
            return Requirements.GetSize() / static_cast<double>(vmDescr.GetFlops());
        }

        double GetExecutionCost(const VMDescription& vmDescr) const {
            return vmDescr.GetPrice() * GetExecutionTime(vmDescr);
        }

        friend class Workflow::AbstractGraph;
    };
}
