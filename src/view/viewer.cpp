#include "viewer.h"

using std::vector;
using std::string;

namespace View {
    const Task& Viewer::GetTaskByName(const std::string& s) const {
        return Pumpkin[0];
    }
    const Task& Viewer::GetTaskById(int id) const {
        return Pumpkin[0];
    }
    const std::vector<Task>& Viewer::GetTaskList() const {
        return Pumpkin;
    }
    const VMList& Viewer::GetAvailiableVMTaxes() const {
        return Simulator.AvailiableVMs;
    }

    const Schedule& Viewer::GetCurrentAssignments() const {
        return Simulator.Assignments;
    }
}

