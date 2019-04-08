#include "platform/platform.h"
#include <simgrid/s4u.hpp>
#include "spec.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(platform_cloud_platform, "cloud platform log");

namespace {
    void DoExecute(double flops) {
        simgrid::s4u::Host* host = simgrid::s4u::this_actor::get_host();

        double timeStart = simgrid::s4u::Engine::get_clock();
        host->execute(flops);
        double timeFinish = simgrid::s4u::Engine::get_clock();


        XBT_INFO("%s: actor %s executed %g seconds", host->get_cname(),
                simgrid::s4u::this_actor::get_cname(), timeFinish - timeStart);
        simgrid::s4u::this_actor::exit();
    }
}

bool CloudPlatform::CreateVM(int hostId, const ComputeSpec& s, int id) {
    if (VirtualMachines.count(id)) {
        return false;
    }
    if (!HostsList[hostId].CreateVM(s, id)) {
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
        //XBT_INFO("%d %d %d \/ %d %d %d", host.Spec.Cores, host.Spec.Memory, host.Spec.Speed, s.Cores, s.Memory, s.Speed);
        if (host.VirtualMachines.empty()/*FIXME && host.Spec == s*/) {
            return host.Id;
        }
    }
    return -1;
}
