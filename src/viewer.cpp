#include "viewer.h"

using std::vector;
using std::string;

int Viewer::GetTaskId(const string& s) const {
    return 0;
}

vector<int> Viewer::GetTaskParents(int id) const {
    return vector<int>();
}

vector<int> Viewer::GetTaskChildren(int id) const {
    return vector<int>();
}

string Viewer::GetTaskName(int id) const {
    return "Test Name";
}

ComputeSpec Viewer::GetTaskRequirements(int id) const {
    return {0, 0, 0};
}

ComputeSpec Viewer::GetTaskSize(int id) const {
    return {0, 0, 0};
}

