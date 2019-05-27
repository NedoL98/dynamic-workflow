#include "common/parse_log.h"
#include "events/event.h"
#include <map>
#include <fstream>
#include <iostream>
#include <vector>

using std::string;
using std::map;


RunSpec ParseLogFile(const string& filename) {
    /*
        Function to parse event.log generated by simulation
        returns ParseSpec from spec.h
    */

    std::ifstream log(filename);
    string currentString;
    map<string, bool> isActive, isExecuting;
    map<string, double> activeTime, executingTime;
    map<string, double> firstMention, lastMention;
    map<string, double> costs;
    YAML::Node previousEvent;
    previousEvent["time"] = 0;
    while (std::getline(log, currentString)) {
        string currentEvent;
        while (!currentString.empty()) {
            currentEvent += currentString;
            currentEvent.push_back('\n');
            std::getline(log, currentString);
        }
        YAML::Node eventNode = YAML::Load(currentEvent);
        for (const auto& [name, value] : isActive) {
            if (value) {
                activeTime[name] += eventNode["time"].as<double>() - previousEvent["time"].as<double>();
            }
        }
        for (const auto& [name, value] : isExecuting) {
            if (value) {
                executingTime[name] += eventNode["time"].as<double>() - previousEvent["time"].as<double>();
            }
        }
        string currentName = eventNode["host"]["name"].as<string>();

        if (currentName != "NONE") {
            EventType type = eventNode["type"].as<EventType>();
            if (type == EventType::TaskStarted || type == EventType::TransferStarted) {
                isActive[currentName] = true;
            }
            if (type == EventType::TaskStarted) {
                isExecuting[currentName] = true;
            } else if (type == EventType::TaskFinished) {
                isExecuting[currentName] = false;
            }
            if (!firstMention.count(currentName)) {
                firstMention[currentName] = eventNode["time"].as<double>();
            }
            lastMention[currentName] = eventNode["time"].as<double>();
            costs[currentName] = eventNode["host"]["vm_cost"].as<double>();
        }
        previousEvent = YAML::Clone(eventNode);
    }
    RunSpec result = {};
    result.Time = previousEvent["time"].as<double>();
    for (auto& [name, value] : activeTime) {
        value -= previousEvent["time"].as<double>() - lastMention[name];
        result.Cost += value * costs[name];
        VMRunSpec currentHostSpec({
            name,
            costs[name],
            firstMention[name],
            lastMention[name],
            executingTime[name]
        });

        result.VMRuns.push_back(currentHostSpec);
    }

    return result;
}
