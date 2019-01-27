#pragma once

#include <vector>
#include <memory>
#include <map>

#include "platform/host.h"
#include "spec.h"

class AbstractPlatform {
    std::vector<Host> HostsList;
    
public:
    AbstractPlatform(const std::string& platformConfig);
    virtual bool CreateVM(int hostId, const ComputeSpec& s) = 0;
    virtual bool AssignJob(int hostId, int vmId, long long cost) = 0;
};


