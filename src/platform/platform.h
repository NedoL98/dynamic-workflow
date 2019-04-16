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
    AbstractPlatform(const std::string& platformConfig);
    std::vector<int> GetVMIds() const; 
    virtual bool CreateVM(int hostId, const VMDescription& s, int id) = 0;
    virtual simgrid::s4u::ActorPtr AssignTask(int vmId, const TaskSpec& s, const std::function<void(int, void*)>& onExit, void* arg) = 0;
    virtual void FinishTask(int hostId, const TaskSpec& spec) = 0;
};


class CloudPlatform : public AbstractPlatform {

public:
    CloudPlatform(const std::string& platformConfig): AbstractPlatform(platformConfig) {}

    int GetEmptyHost(const ComputeSpec& s);
    bool CheckTask(int vmId, const TaskSpec& s);

// AbstractPlatform
    virtual bool CreateVM(int hostId, const VMDescription& s, int id) override;
    virtual simgrid::s4u::ActorPtr AssignTask(int vmId, const TaskSpec& s, const std::function<void(int, void*)>& onExit, void* arg) override;
    virtual void FinishTask(int hostId, const TaskSpec& spec) override;
};

