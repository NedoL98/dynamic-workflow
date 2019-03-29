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
        return *Simulator.TaskGraph.Nodes[Simulator.TaskGraph.TaskName2Id.find(s)->second];
    }
    const Task& Viewer::GetTaskById(int id) const {
        return *Simulator.TaskGraph.Nodes[id];
    }
    const std::vector<std::unique_ptr<Task>>& Viewer::GetTaskList() const {
        return Simulator.TaskGraph.Nodes;
    }
    const VMList& Viewer::GetAvailiableVMTaxes() const {
        return Simulator.AvailiableVMs;
    }

    const Schedule& Viewer::GetCurrentAssignments() const {
        return Simulator.Assignments;
    }

    double Viewer::GetDeadline() const {
        return Simulator.TaskGraph.Deadline;
    }
}

