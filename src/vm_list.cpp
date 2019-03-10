#include "vm_list.h"

#include "common/common.h"
#include <simgrid/s4u/VirtualMachine.hpp>
#include <yaml-cpp/yaml.h>

using std::string;
using std::vector;

XBT_LOG_NEW_DEFAULT_CATEGORY(vm_list, "VM list log");

VMDescription::VMDescription(int id, int cores, double memory, double flops, int price):
    Id(id),
    Cores(cores),
    Memory(memory),
    Flops(flops),
    Price(price)
{}

int VMDescription::GetId() const {
    return Id;
}

int VMDescription::GetCores() const {
    return Cores;
}

double VMDescription::GetMemory() const {
    return Memory;
}

double VMDescription::GetFlops() const {
    return Flops;
}

int VMDescription::GetPrice() const {
    return Price;
}

bool VMDescription::operator ==(const VMDescription& other) const {
    return Cores == other.Cores &&
           Memory == other.Memory &&
           Flops == other.Flops &&
           Price == other.Price;           
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

simgrid::s4u::VirtualMachine* VMList::GetVMInstance(const string& taskName, int vmId) const {
    VMDescription currentVM = VMs[vmId];
    int cores = currentVM.GetCores();
    double memory = currentVM.GetMemory();

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
    return vm;
}

int VMList::MaxCores() const {
    return std::max_element(VMs.begin(), VMs.end(), 
        [] (const auto& lhs, const auto& rhs) {
            return lhs.GetCores() < rhs.GetCores();
        })->GetCores();
}

double VMList::MaxMemory() const {
    return std::max_element(VMs.begin(), VMs.end(), 
        [] (const auto& lhs, const auto& rhs) {
            return lhs.GetMemory() < rhs.GetMemory();
        })->GetMemory();
}

VMList::VMList(const string& vmConfig) {
    XBT_INFO("Loading VM list from %s", vmConfig.c_str());
    YAML::Node vmList = YAML::LoadFile(vmConfig);

    int id = 0;
    for (const YAML::Node& vmDescription: vmList) {
        int cores = vmDescription["cpu"].as<int>();
        double memory = ParseSize(vmDescription["memory"].as<string>(), SizeSuffixes);
        double flops = ParseSize(vmDescription["speed"].as<string>(), PerformanceSuffixes);
        int price = vmDescription["price"].as<int>();
        VMs.push_back(VMDescription(id, cores, memory, flops, price));
        ++id;
        XBT_INFO("VM %s loaded!", vmDescription["name"].as<string>().c_str());
    }
}