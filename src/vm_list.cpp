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

TaskSpec::TaskSpec(const YAML::Node& taskDescription) {
    if (!taskDescription["requirements"]) {
        XBT_DEBUG("Task requirements are not specified, they will be set to default!");
        Cores = DefaultCores;
        Memory = DefaultMemory;
    } else {
        if (!taskDescription["requirements"]["cpu"]) {
            XBT_DEBUG("CPU usage is not specified for task, it will be set to default!");
            Cores = DefaultCores;
        } else {
            Cores = taskDescription["requirements"]["cpu"].as<int>();
        }

        if (!taskDescription["requirements"]["memory"]) {
            XBT_DEBUG("Memory usage is not specified for task, it will be set to default!");
            Memory = DefaultMemory;
        } else {
            try {
                Memory = ParseSize(taskDescription["requirements"]["memory"].as<string>(), SizeSuffixes);
            } catch (std::exception& e) {
                XBT_ERROR("Can't parse memory requirement: %s", e.what());
                XBT_DEBUG("Memory requirement will be set to %d", DefaultMemory);
                Memory = DefaultMemory;
            }
        }
    }

    xbt_assert(taskDescription["size"], "Task size must be specified!");
    try {
        Size = ParseSize(taskDescription["size"].as<string>(), PerformanceSuffixes);
    } catch (std::exception& e) {
        xbt_assert("Can't parse size requirement: %s", e.what());
    }
}

long long TaskSpec::GetMemory() const {
    return Memory;
}

long long TaskSpec::GetSize() const {
    return Size;
}

int TaskSpec::GetCores() const {
    return Cores;
}

int VMDescription::GetId() const {
    return Id;
}

int VMDescription::GetCores() const {
    return Cores;
}

long long VMDescription::GetMemory() const {
    return Memory;
}

long long VMDescription::GetFlops() const {
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

double VMList::GetMinLag() const {
    return MinLag;
}

double VMList::GetMaxLag() const {
    return MaxLag;
}

VMList::VMList(const string& vmConfig) {
    XBT_INFO("Loading VM list from %s", vmConfig.c_str());
    YAML::Node VMList_ = YAML::LoadFile(vmConfig);
    
    int id = 0;
    for (const YAML::Node& vmDescription: VMList_["tasks"]) {
        int cores = vmDescription["cpu"].as<int>();
        long long memory = ParseSize(vmDescription["memory"].as<string>(), SizeSuffixes);
        long long flops = ParseSize(vmDescription["speed"].as<string>(), PerformanceSuffixes);
        int price = vmDescription["price"].as<int>();
        int pStateId = vmDescription["pstate_id"].as<int>();

        VMs.push_back(VMDescription(id, cores, memory, flops, price, pStateId));
        ++id;
        XBT_INFO("VM %s loaded!", vmDescription["name"].as<string>().c_str());
    }

    if (!VMList_["acquisition_lag"]) {
        MinLag = 0;
        MaxLag = 0;
    } else {
        MinLag = VMList_["acquisition_lag"]["min"].as<double>();
        MaxLag = VMList_["acquisition_lag"]["max"].as<double>();
    }

    XBT_INFO("MinLag %f, MaxLag %f", MinLag, MaxLag);
}
