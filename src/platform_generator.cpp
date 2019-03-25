#include "platform_generator.h"

#include <iostream>
#include <sys/wait.h>

using std::string;
using std::stringstream;
using std::vector;

string GetHostSpeeds(const vector<double>& vmSpeeds) {
    string result = "";

    for (double vmSpeed: vmSpeeds) {
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

    vector<double> vmSpeeds;
    for (const VMDescription& vmDescription: vmList) {
        vmSpeeds.push_back(vmDescription.GetFlops() / (1000 * 1000 * 1000));
    }

    string hostSpeeds = GetHostSpeeds(vmSpeeds);

    hostCnt = tasksGraph.Size();
    maxCoresCnt = vmList.MaxCores();
    maxMemory = vmList.MaxMemory();

    int pid = fork();
    if (pid > 0) {
        int wstatus;
        waitpid(pid, &wstatus, 0);
    } else {
        execlp("python3", "python3", "../tools/plat_gen.py", "../resources", "1", "0", "cluster", 
                std::to_string(hostCnt).c_str(), 
                hostSpeeds.c_str(),
                std::to_string(maxCoresCnt).c_str(), 
                "1", 
                "1", 
                nullptr);
    }

    std::stringstream platformPathBuilder;

    platformPathBuilder << "../resources/cluster"
                        << "_" << std::to_string(hostCnt)
                        << "_" << std::to_string(maxCoresCnt)
                        << "_" << std::to_string(1)
                        << "_" << std::to_string(1)
                        << "_0.xml";
    return platformPathBuilder.str(); 
}