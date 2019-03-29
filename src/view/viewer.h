#pragma once
#include <prototypes/simulator.h>
#include <vector>
#include <string>
#include <spec.h>
#include <view/task_view.h>
#include <view/status.h>
#include <vm_list.h>

namespace View {
    class Viewer {
        const CloudSimulator& Simulator;
        std::vector<Task> Tasks;

    public:
        Viewer(const CloudSimulator& s);
        
        const Task& GetTaskByName(const std::string& s) const;
        const Task& GetTaskById(int id) const;
        const std::vector<std::unique_ptr<Task>>& GetTaskList() const;
        const VMList& GetAvailiableVMTaxes() const;
        const Schedule& GetCurrentAssignments() const;
        Status GetTaskStatus(int id) const;
        double GetDeadline() const;
    };
}
