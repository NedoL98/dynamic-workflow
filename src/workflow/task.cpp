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

        if (!taskDescription["requirements"]) {
            XBT_DEBUG("Task requirements are not specified, they will be set to default!");
            Requirements.Cores = DefaultCores;
            Requirements.Memory = DefaultMemory;
        } else {
            if (!taskDescription["requirements"]["cpu"]) {
                XBT_DEBUG("CPU usage is not specified for task, it will be set to default!");
                Requirements.Cores = DefaultCores;
            } else {
                Requirements.Cores = taskDescription["requirements"]["cpu"].as<int>();
            }

            if (!taskDescription["requirements"]["memory"]) {
                XBT_DEBUG("Memory usage is not specified for task, it will be set to default!");
                Requirements.Memory = DefaultMemory;
            } else {
                try {
                    Requirements.Memory = ParseSize(taskDescription["requirements"]["memory"].as<string>(), SizeSuffixes);
                } catch (std::exception& e) {
                    XBT_ERROR("Can't parse memory requirement: %s", e.what());
                    XBT_DEBUG("Memory requirement will be set to %d", DefaultMemory);
                    Requirements.Memory = DefaultMemory;
                }
            }
        }

        xbt_assert(taskDescription["size"], "Task size must be specified!");
        try {
            Requirements.Cost = ParseSize(taskDescription["size"].as<string>(), PerformanceSuffixes);
        } catch (std::exception& e) {
            xbt_assert("Can't parse size requirement: %s", e.what());
        }
    }
}
