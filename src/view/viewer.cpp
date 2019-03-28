#include "viewer.h"
#include <map>

using std::vector;
using std::string;
using std::map;

namespace View {
    Viewer::Viewer(const CloudSimulator& s):
        Simulator(s) {
        for (int nodeId = 0; nodeId < Simulator.TaskGraph.Nodes.size(); nodeId++) {
            Tasks.push_back(Task(nodeId, *Simulator.TaskGraph.Nodes[nodeId]));
        }
    }
    const Task& Viewer::GetTaskByName(const std::string& s) const {
        return Tasks[Simulator.TaskGraph.TaskName2Id.find(s)->second];
    }
    const Task& Viewer::GetTaskById(int id) const {
        return Tasks[id];
    }
    const std::vector<Task>& Viewer::GetTaskList() const {
        return Tasks;
    }
    const VMList& Viewer::GetAvailiableVMTaxes() const {
        return Simulator.AvailiableVMs;
    }

    const Schedule& Viewer::GetCurrentAssignments() const {
        return Simulator.Assignments;
    }
}

