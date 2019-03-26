#include "platform_generator.h"

#include <iostream>
#include <sys/wait.h>

using std::pair;
using std::string;
using std::stringstream;
using std::vector;

XBT_LOG_NEW_DEFAULT_CATEGORY(platform_generator, "Platform generator log");

string GetHostSpeeds(const vector<pair<int, double>>& vmSpeeds) {
    string result = "";

    for (const auto& [pStateId, vmSpeed]: vmSpeeds) {
        if (!result.empty()) {
            result += ",";
        }
        result += std::to_string(vmSpeed);
    }

    return result;
}

string GeneratePlatform(const string& workflowPath, const string& vmListPath) {
    int hostCnt;
    int maxCoresCnt;
    // TODO: use this parameter while generating platform
    double maxMemory;

    TasksGraph tasksGraph(workflowPath);
    VMList vmList(vmListPath);

    vector<pair<int, double>> vmSpeeds;
    vector<bool> usedPstateId(vmList.Size());
    for (const VMDescription& vmDescription: vmList) {
        xbt_assert(vmDescription.GetPStateId() < vmList.Size(), "PStateId %d is not in bounds [0,%d)!", vmDescription.GetPStateId(), vmList.Size());
        xbt_assert(!usedPstateId[vmDescription.GetPStateId()], "PStateId %d is used twice!", vmDescription.GetPStateId());
        vmSpeeds.push_back({vmDescription.GetPStateId(), vmDescription.GetFlops() / (1000 * 1000 * 1000)});
        usedPstateId[vmDescription.GetPStateId()] = true;
    }
    std::sort(vmSpeeds.begin(), vmSpeeds.end());

    string hostSpeeds = GetHostSpeeds(vmSpeeds);

    hostCnt = tasksGraph.Size();
    maxCoresCnt = vmList.MaxCores();
    maxMemory = vmList.MaxMemory();

    int pid = fork();
    if (pid > 0) {
        int wstatus;
        waitpid(pid, &wstatus, 0);
    } else {
        execlp("python3", 
                "python3",
                (string(PROJECT_PATH) + "/tools/plat_gen.py").c_str(), 
                (string(PROJECT_PATH) + "/resources").c_str(), 
                "1", 
                "0", 
                "cluster", 
                std::to_string(hostCnt).c_str(), 
                hostSpeeds.c_str(),
                std::to_string(maxCoresCnt).c_str(), 
                "1", 
                "1", 
                nullptr);
    }

    std::stringstream platformPathBuilder;

    platformPathBuilder << string(PROJECT_PATH) + "/resources/cluster"
                        << "_" << std::to_string(hostCnt)
                        << "_" << std::to_string(maxCoresCnt)
                        << "_" << std::to_string(1)
                        << "_" << std::to_string(1)
                        << "_0.xml";
    return platformPathBuilder.str(); 
}