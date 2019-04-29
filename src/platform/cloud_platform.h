#pragma once

#include <vector>
#include <memory>
#include <map>

#include "platform/platform.h"
#include "spec.h"
#include "vm_list.h"


class CloudPlatform : public AbstractPlatform {
    double MinStartupTime, MaxStartupTime;
    std::function<void(TransferSpec*)> OnTransferFinish;
    std::map<int, simgrid::s4u::ActorPtr> Mailers; // only for one purpose - not letting simgrid destroy our actor
public:
    CloudPlatform(const std::string& platformConfig, const VMList& vmList, std::function<void(TransferSpec*)> onTransferFinish)
                            : AbstractPlatform(platformConfig, vmList.MaxMemory())
                            , MinStartupTime(vmList.GetMinLag())
                            , MaxStartupTime(vmList.GetMaxLag())
                            , OnTransferFinish(onTransferFinish) {
    }

    ~CloudPlatform();

    int GetEmptyHost(const ComputeSpec& s);
    bool CheckTask(int vmId, const TaskSpec& s);

// AbstractPlatform
    virtual bool CreateVM(int hostId, const VMDescription& s, int id) override;
    virtual simgrid::s4u::ActorPtr AssignTask(int vmId, const TaskSpec& s, const std::function<void(int, void*)>& onExit, void* arg) override;
    virtual void FinishTask(int hostId, const TaskSpec& spec) override;
    virtual void StartTransfer(const TransferSpec& file) override;
};

