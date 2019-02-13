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

bool CloudPlatform::CreateVM(int hostId, const ComputeSpec& s, int& id) {
    if (HostsList[hostId].CreateVM(s) == -1) {
        return false;
    }
    id = VirtualMachines.size();
    VirtualMachines.push_back(HostsList[hostId].VirtualMachines.back());
    VirtualMachineSpecs.push_back(s);
    return true;
}

simgrid::s4u::ActorPtr CloudPlatform::AssignTask(int vmId, const TaskSpec& requirements) {
    if (vmId < 0 || vmId >= VirtualMachines.size()) {
        return nullptr;
    }
    if (VirtualMachineSpecs[vmId].Cores < requirements.Cores || 
        VirtualMachineSpecs[vmId].Memory < requirements.Memory) {
        return nullptr;
    }
    simgrid::s4u::ActorPtr result = simgrid::s4u::Actor::create("compute", VirtualMachines[vmId], DoExecute, requirements.Cost);
    VirtualMachineSpecs[vmId].Cores -= requirements.Cores;
    VirtualMachineSpecs[vmId].Memory -= requirements.Memory;
    return result;
}

