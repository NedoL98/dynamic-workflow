#pragma once
#include <prototypes/cloud_simulator.h>
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
        Workflow::AbstractGraph::GraphAbstractIterator* GetGraphIterator();
        const VMList& GetVMList() const;
        const Schedule& GetCurrentAssignments() const;
        Status GetTaskStatus(int id) const;
        double GetDeadline() const;
        std::vector<Task> MakeTasksOrder() const;
        size_t WorkflowSize() const;
    };
}
