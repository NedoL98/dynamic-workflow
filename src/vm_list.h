#pragma once

#include <string>
#include <vector>

class VMDescription {
public:
    VMDescription(int cores, double memory, double flops, int price);

    int GetCores() const;
    double GetMemory() const;
    double GetFlops() const;
    int GetPrice() const;

private:
    int Cores;
    double Memory;
    double Flops;
    int Price;
};

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