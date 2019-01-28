#pragma once

#include <string>
#include <vector>

class VMDescription {
public:
    VMDescription(int cores, double memory, double flops, int price);

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

private:
    std::vector<VMDescription> VMs;
};