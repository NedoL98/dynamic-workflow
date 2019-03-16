#include "platform/platform.h"
#include "spec.h"

namespace {
    void DoExecute(double flops) {
        simgrid::s4u::Host* host = simgrid::s4u::this_actor::get_host();

        double timeStart = simgrid::s4u::Engine::get_clock();
        host->execute(flops);
        double timeFinish = simgrid::s4u::Engine::get_clock();


        simgrid::s4u::this_actor::exit();
    }
}

bool CloudPlatform::CreateVM(int hostId, const ComputeSpec& s, int id) {
    if (VirtualMachines.count(id)) {
        return false;
    }
    if (HostsList[hostId].CreateVM(s, id) == -1) {
        return false;
    }
    VirtualMachines[id] = HostsList[hostId].VirtualMachines[id];
    VirtualMachineSpecs[id] = s;
    return true;
}

bool CloudPlatform::CheckTask(int vmId, const TaskSpec& requirements) {
    if (!VirtualMachines.count(vmId)) {
        return false; 
    }
    if (VirtualMachineSpecs[vmId].Cores < requirements.Cores || 
        VirtualMachineSpecs[vmId].Memory < requirements.Memory) {
        return false;
    }
    return true;
}

simgrid::s4u::ActorPtr CloudPlatform::AssignTask(int vmId, const TaskSpec& requirements) {
    if (!CheckTask(vmId, requirements)) {
        return nullptr;
    }
    simgrid::s4u::ActorPtr result = simgrid::s4u::Actor::create("compute", VirtualMachines[vmId], DoExecute, requirements.Cost);
    VirtualMachineSpecs[vmId].Cores -= requirements.Cores;
    VirtualMachineSpecs[vmId].Memory -= requirements.Memory;
    return result;
}

int CloudPlatform::GetEmptyHost(const ComputeSpec& s) {
    for (const auto& host : HostsList) {
        if (host.VirtualMachines.empty() && host.Spec == s) {
            return host.Id;
        }
    }
    return -1;
}
