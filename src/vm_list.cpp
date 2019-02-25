#include "vm_list.h"

#include "common/common.h"
#include <simgrid/s4u.hpp>
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