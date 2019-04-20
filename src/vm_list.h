#pragma once

#include <simgrid/s4u.hpp>
#include <string>
#include <vector>
#include "spec.h"

class VMList {
public:
    VMList() = default;
    VMList(const std::string& vmConfig);

    std::vector<VMDescription>::iterator begin() { return VMs.begin(); };
    std::vector<VMDescription>::iterator end() { return VMs.end(); };
    std::vector<VMDescription>::const_iterator begin() const { return VMs.begin(); };
    std::vector<VMDescription>::const_iterator end() const { return VMs.end(); };

    size_t Size() const {
        return VMs.size();
    };

    // simgrid::s4u::VirtualMachine* GetVMInstance(const std::string& taskName, int vmId) const;

    int MaxCores() const;
    long long MaxMemory() const;

    double GetMinLag() const;
    double GetMaxLag() const;

private:
    std::vector<VMDescription> VMs;
    double MinLag;
    double MaxLag;
};
