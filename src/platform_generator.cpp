#include "platform_generator.h"

#include <iostream>
#include <sys/wait.h>

string GeneratePlatform(BaseScheduler& scheduler) {
    int hostCnt;
    int maxCoresCnt;
    double maxMemoryCnt;

    scheduler.GetMaxParams(hostCnt, maxCoresCnt, maxMemoryCnt);

    string bashCmd = "python3 ../tools/plat_gen.py ../resources 1 0 cluster";
    bashCmd += " " + std::to_string(hostCnt);
    bashCmd += " " + std::to_string(1);
    bashCmd += " " + std::to_string(maxCoresCnt);
    bashCmd += " " + std::to_string(1);
    bashCmd += " " + std::to_string(1);

    int pid = fork();
    if (pid > 0) {
        int wstatus;
        waitpid(pid, &wstatus, 0);
    } else {
        execl("/bin/sh", "sh", "-c", bashCmd.c_str(), (char *) 0);
    }

    string platformPath = "../resources/cluster";
    platformPath += "_" + std::to_string(hostCnt);
    platformPath += "_" + std::to_string(1);
    platformPath += "_" + std::to_string(maxCoresCnt);
    platformPath += "_" + std::to_string(1);
    platformPath += "_" + std::to_string(1);
    platformPath += "_0.xml";
    return platformPath;
}