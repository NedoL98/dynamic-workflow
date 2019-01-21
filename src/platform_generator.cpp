#include "platform_generator.h"

#include <iostream>
#include <sys/wait.h>

using std::stringstream;

string GeneratePlatform(BaseScheduler& scheduler) {
    int hostCnt;
    int maxCoresCnt;
    double maxMemoryCnt;

    scheduler.GetMaxParams(hostCnt, maxCoresCnt, maxMemoryCnt);

    int pid = fork();
    if (pid > 0) {
        int wstatus;
        waitpid(pid, &wstatus, 0);
    } else {
        execlp("python3", "python3", "../tools/plat_gen.py", "../resources", "1", "0", "cluster", 
                std::to_string(hostCnt).c_str(), "1", std::to_string(maxCoresCnt).c_str(), "1", "1", nullptr);
    }

    std::stringstream platformPathBuilder;

    platformPathBuilder << "../resources/cluster"
                        << "_" << std::to_string(hostCnt)
                        << "_" << std::to_string(1)
                        << "_" << std::to_string(maxCoresCnt)
                        << "_" << std::to_string(1)
                        << "_" << std::to_string(1)
                        << "_0.xml";
    return platformPathBuilder.str(); 
}