#include <common/common.h>
#include <workflow/task.h>
#include <workflow/file.h>
#include <map>
#include <string>
#include <vector>
#include <simgrid/s4u.hpp>

using std::map;
using std::vector;
using std::string;

XBT_LOG_NEW_DEFAULT_CATEGORY(workflow_task_cpp, "task log");

namespace Workflow {
    Task::Task(const YAML::Node& taskDescription, const FileRegistry& registry, int id):
        Requirements(taskDescription),
        Id(id) {
        Name = taskDescription["name"].as<string>();
        for (const YAML::Node& inputDescription: taskDescription["inputs"]) {
            string inputName = inputDescription["name"].as<string>();
            Inputs.push_back(registry.find(inputName)->second.Id);
        }
        for (const YAML::Node& outputDescription: taskDescription["outputs"]) {
            string outputName = outputDescription["name"].as<string>();
            Outputs.push_back(registry.find(outputName)->second.Id);
        }
    }
}
