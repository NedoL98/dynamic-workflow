#include "vm_list.h"

#include "common/common.h"
#include <simgrid/s4u.hpp>
#include <yaml-cpp/yaml.h>

using std::string;
using std::vector;

XBT_LOG_NEW_DEFAULT_CATEGORY(vm_list, "VM list log");

VMDescription::VMDescription(int cores, double memory, double flops, int price):
    Cores(cores),
    Memory(memory),
    Flops(flops),
    Price(price)
{}

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

VMList::VMList(const string& vmConfig) {
    XBT_INFO("Loading VM list from %s", vmConfig.c_str());
    YAML::Node vmList = YAML::LoadFile(vmConfig);

    for (const YAML::Node& vmDescription: vmList) {
        int cores = vmDescription["cpu"].as<int>();
        double memory = ParseSize(vmDescription["memory"].as<string>(), SizeSuffixes);
        double flops = ParseSize(vmDescription["speed"].as<string>(), PerformanceSuffixes);
        int price = vmDescription["price"].as<int>();
        VMs.push_back(VMDescription(cores, memory, flops, price));
        XBT_INFO("VM %s loaded!", vmDescription["name"].as<string>().c_str());
    }
}