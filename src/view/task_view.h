#pragma once
#include <vector>
#include <spec.h>

namespace {
    int GetUniqueTaskId() {
        static int id = 0;
        return id++;
    }
}

namespace View {
    class Task {
        std::vector<Task> Dependencies, Successors;
        ComputeSpec Spec;
        int id;
    public:
        Task() {
            id = GetUniqueTaskId();
        }

        const std::vector<Task>& GetDependencies() const {
            return Dependencies;
        }
        const std::vector<Task>& GetSuccessors() const {
            return Successors;
        }
        const ComputeSpec& GetComputeSpec() const {
            return Spec;
        }
    };
}
