#pragma once

#include <vector>
#include <memory>
#include <map>

#include "platform/host.h"
#include "spec.h"

class AbstractPlatform {
protected:
    std::vector<Host> HostsList;
    std::map<int, simgrid::s4u::VirtualMachine *> VirtualMachines;
    std::map<int, ComputeSpec> VirtualMachineSpecs;
    
public:
    AbstractPlatform(const std::string& platformConfig, long long hostMaxMemory);
    std::vector<int> GetVMIds() const; 
    virtual bool CreateVM(int hostId, const VMDescription& s, int id) = 0;
    virtual simgrid::s4u::ActorPtr AssignTask(int vmId, const TaskSpec& s, const std::function<void(int, void*)>& onExit, void* arg) = 0;
    virtual void FinishTask(int hostId, const TaskSpec& spec) = 0;
};
