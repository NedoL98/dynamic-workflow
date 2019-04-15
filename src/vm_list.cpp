#include "vm_list.h"

#include "common/common.h"
#include <simgrid/s4u/VirtualMachine.hpp>
#include <yaml-cpp/yaml.h>

using std::string;
using std::vector;

XBT_LOG_NEW_DEFAULT_CATEGORY(vm_list, "VM list log");

VMDescription::VMDescription(int id, int cores, long long memory, long long flops, int price, int pStateId):
    Id(id),
    Cores(cores),
    Memory(memory),
    Flops(flops),
    Price(price),
    PStateId(pStateId)
{}

int VMDescription::GetId() const {
    return Id;
}

int VMDescription::GetCores() const {
    return Cores;
}

long long VMDescription::GetMemory() const {
    return Memory;
}

double VMDescription::GetFlops() const {
    return Flops;
}

int VMDescription::GetPrice() const {
    return Price;
}

int VMDescription::GetPStateId() const {
    return PStateId;
}

ComputeSpec VMDescription::GetSpec() const {
    return ComputeSpec({Memory, Flops, Cores});
}

bool VMDescription::operator ==(const VMDescription& other) const {
    return Cores == other.Cores &&
           Memory == other.Memory &&
           Flops == other.Flops &&
           Price == other.Price &&
           PStateId == other.PStateId;           
}

bool VMDescription::operator !=(const VMDescription& other) const {
    return !(*this == other);
}

bool VMDescription::operator <(const VMDescription& other) const {
    return Flops < other.Flops;
}

bool VMDescription::operator >(const VMDescription& other) const {
    return Flops > other.Flops;
}

bool VMDescription::operator <=(const VMDescription& other) const {
    return !(*this > other);
}

bool VMDescription::operator >=(const VMDescription& other) const {
    return !(*this < other);
}

/*
simgrid::s4u::VirtualMachine* VMList::GetVMInstance(const string& taskName, int vmId) const {
    VMDescription currentVM = VMs[vmId];
    int cores = currentVM.GetCores();
    long long memory = currentVM.GetMemory();

    static int hostId = 0;
    string hostIdStr = std::to_string(hostId);

    simgrid::s4u::Engine *e = simgrid::s4u::Engine::get_instance();
    simgrid::s4u::Host* host = nullptr;
    std::vector<simgrid::s4u::Host*> allHosts = e->get_all_hosts();
    
    for (simgrid::s4u::Host* curHost: allHosts) {
        if (curHost->get_name() == "host" + hostIdStr) {
            host = curHost;
            break;
        }
    }
    xbt_assert(host != nullptr, "Appropriate host for running the task not found!");
    ++hostId;

    XBT_INFO("Host found: %s", host->get_cname());

    simgrid::s4u::VirtualMachine* vm = new simgrid::s4u::VirtualMachine(taskName + "_VM", host, cores, memory);
    vm->set_pstate(currentVM.GetPStateId());
    return vm;
}
*/

int VMList::MaxCores() const {
    return std::max_element(VMs.begin(), VMs.end(), 
        [] (const auto& lhs, const auto& rhs) {
            return lhs.GetCores() < rhs.GetCores();
        })->GetCores();
}

long long VMList::MaxMemory() const {
    return std::max_element(VMs.begin(), VMs.end(), 
        [] (const auto& lhs, const auto& rhs) {
            return lhs.GetMemory() < rhs.GetMemory();
        })->GetMemory();
}

VMList::VMList(const string& vmConfig) {
    XBT_INFO("Loading VM list from %s", vmConfig.c_str());
    YAML::Node VMList_ = YAML::LoadFile(vmConfig);

    int id = 0;
    for (const YAML::Node& vmDescription: VMList_) {
        int cores = vmDescription["cpu"].as<int>();
        long long memory = ParseSize(vmDescription["memory"].as<string>(), SizeSuffixes);
        long long flops = ParseSize(vmDescription["speed"].as<string>(), PerformanceSuffixes);
        int price = vmDescription["price"].as<int>();
        int pStateId = vmDescription["pstate_id"].as<int>();

        VMs.push_back(VMDescription(id, cores, memory, flops, price, pStateId));
        ++id;
        XBT_INFO("VM %s loaded!", vmDescription["name"].as<string>().c_str());
    }
}
