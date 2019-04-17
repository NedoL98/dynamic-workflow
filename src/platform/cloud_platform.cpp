#include "platform/platform.h"
#include <simgrid/s4u.hpp>
#include "spec.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(platform_cloud_platform, "cloud platform log");

namespace {
    void DoExecute(long long flops, const std::function<void(int, void*)>& onExit, void* arg) {
        simgrid::s4u::Host* host = simgrid::s4u::this_actor::get_host();

        double timeStart = simgrid::s4u::Engine::get_clock();
        host->execute(flops);
        double timeFinish = simgrid::s4u::Engine::get_clock();


        XBT_INFO("%s: actor %s executed %g seconds", host->get_cname(),
                simgrid::s4u::this_actor::get_cname(), timeFinish - timeStart);
        onExit(0, arg);
        simgrid::s4u::this_actor::exit();
    }
}

bool CloudPlatform::CreateVM(int hostId, const VMDescription& s, int id) {
    if (VirtualMachines.count(id)) {
        return false;
    }
    if (!HostsList[hostId].CreateVM(s, id)) {
        return false;
    }
    VirtualMachines[id] = HostsList[hostId].VirtualMachines[id];
    VirtualMachineSpecs[id] = s.GetSpec();
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

simgrid::s4u::ActorPtr CloudPlatform::AssignTask(int vmId, const TaskSpec& requirements, const std::function<void(int, void*)>& onExit, void* args) {
    static int i = 0;
    if (!CheckTask(vmId, requirements)) {
        XBT_INFO("Does not satisfy requirements, doesn't run!");
        return nullptr;
    }
    simgrid::s4u::ActorPtr result = simgrid::s4u::Actor::create("compute" + std::to_string(i), VirtualMachines[vmId], DoExecute, (long long)requirements.Cost, onExit, args);
    i++;
    VirtualMachineSpecs[vmId].Cores -= requirements.Cores;
    VirtualMachineSpecs[vmId].Memory -= requirements.Memory;
    return result;
}

int CloudPlatform::GetEmptyHost(const ComputeSpec& s) {
    for (const auto& host : HostsList) {
        if (host.VirtualMachines.empty() && host.Spec.Memory >= s.Memory && host.Spec.Cores >= s.Cores) {
            return host.Id;
        }
    }
    return -1;
}

void CloudPlatform::FinishTask(int vmId, const TaskSpec& requirements) {
    VirtualMachineSpecs[vmId].Cores += requirements.Cores;
    VirtualMachineSpecs[vmId].Memory += requirements.Memory;
}
