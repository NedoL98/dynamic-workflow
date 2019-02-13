#pragma once

#include <vector>
#include <memory>
#include <map>

#include "platform/host.h"
#include "spec.h"

class AbstractPlatform {
protected:
    std::vector<Host> HostsList;
    std::vector<simgrid::s4u::VirtualMachine *> VirtualMachines;
    std::vector<ComputeSpec> VirtualMachineSpecs;
    
public:
    AbstractPlatform(const std::string& platformConfig);
    virtual bool CreateVM(int hostId, const ComputeSpec& s, int& id) = 0;
    virtual simgrid::s4u::ActorPtr AssignTask(int vmId, const TaskSpec& s) = 0;
};


class CloudPlatform : public AbstractPlatform {
public:

    CloudPlatform(const std::string& platformConfig);
    virtual bool CreateVM(int hostId, const ComputeSpec& s, int& id) override;
    virtual simgrid::s4u::ActorPtr AssignTask(int vmId, const TaskSpec& s) override;
};
    

