#pragma once

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

private:
    std::vector<VMDescription> VMs;
};
