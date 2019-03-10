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

    int Size() { return VMs.size(); };

    simgrid::s4u::VirtualMachine* GetVMInstance(const std::string& taskName, int vmId) const;

private:
    std::vector<VMDescription> VMs;
};
