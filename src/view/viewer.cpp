#include "viewer.h"
#include <map>
#include <memory>

using std::vector;
using std::string;
using std::map;

namespace View {
    Viewer::Viewer(const CloudSimulator& s):
        Simulator(s) {
            /*
        for (int nodeId = 0; nodeId < Simulator.TaskGraph.Nodes.size(); nodeId++) {
            Tasks.push_back(Task(nodeId, *Simulator.TaskGraph.Nodes[nodeId]));
        }*/
    }
    const Task& Viewer::GetTaskByName(const std::string& s) const {
        return Simulator.TaskGraph->GetTaskByName(s);
    }
    const Task& Viewer::GetTaskById(int id) const {
        return Simulator.TaskGraph->GetTask(id);
    }
    Workflow::AbstractGraph::GraphAbstractIterator* Viewer::GetGraphIterator() {
        return Simulator.TaskGraph->GetGraphIterator();
    }
    const VMList& Viewer::GetVMList() const {
        return Simulator.AvailableVMs;
    }

    const Schedule& Viewer::GetCurrentAssignments() const {
        return Simulator.Assignments;
    }

    double Viewer::GetDeadline() const {
        return dynamic_cast<Workflow::Graph*>(Simulator.TaskGraph)->GetDeadline();
    }

    vector<Task> Viewer::MakeTasksOrder() const {
        return dynamic_cast<Workflow::Graph*>(Simulator.TaskGraph)->MakeTasksOrder();
    }

    size_t Viewer::WorkflowSize() const {
        return Simulator.TaskGraph->GetTaskNumber();
    }
}

