#pragma once
#include <vector>
#include <spec.h>
#include <workflow/task.h>

/*
namespace {
    int GetUniqueTaskId() {
        static int id = 0;
        return id++;
    }
}
*/

namespace View {
    /*
    class Task {
        std::string Name;
        std::vector<int> Dependencies, Successors;
        TaskSpec Spec;
        int Id;
    public:
        Task(int id, const Workflow::Task& task):
            Name(task.Name),
            Dependencies(task.Dependencies), 
            Successors(task.Successors),
            Spec(task.Requirements),
            Id(id) {
        }

        int GetId() const {
            return Id;
        }

        const std::vector<int>& GetDependencies() const {
            return Dependencies;
        }
        const std::vector<int>& GetSuccessors() const {
            return Successors;
        }
        const TaskSpec& TaskSpec() const {
            return Spec;
        }
    };
    */
    typedef Workflow::Task Task;
}
