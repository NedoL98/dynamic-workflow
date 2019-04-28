#include <common/common.h>
#include <workflow/task.h>
#include <map>
#include <string>
#include <vector>
#include <simgrid/s4u.hpp>

using std::map;
using std::vector;
using std::string;

XBT_LOG_NEW_DEFAULT_CATEGORY(workflow_task_cpp, "task log");

namespace Workflow {
    Task::Task(const YAML::Node& taskDescription, const Manager& registry, int id)
        : Requirements(taskDescription)
        , State(EState::NOT_SCHEDULED)
        , Id(id) {
        Name = taskDescription["name"].as<string>();
        for (const YAML::Node& inputDescription: taskDescription["inputs"]) {
            string inputName = inputDescription["name"].as<string>();
            Inputs.push_back(registry.GetFileByName(inputName).Id);
        }
        for (const YAML::Node& outputDescription: taskDescription["outputs"]) {
            string outputName = outputDescription["name"].as<string>();
            Outputs.push_back(registry.GetFileByName(outputName).Id);
        }
    }
}
