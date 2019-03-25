#pragma once
struct ComputeSpec {
    long long Memory, Speed;
    int Cores;
    bool operator ==(const ComputeSpec& other) const {
        return Memory == other.Memory && Speed == other.Speed && Cores == other.Cores;
    }
};

struct TaskSpec {
    long long Memory, Cost;
    int Cores;
};

class VMDescription {
public:
    VMDescription() = default;
    VMDescription(int id, int cores, double memory, double flops, int price, int pStateId);

    int GetId() const;
    int GetCores() const;
    double GetMemory() const;
    double GetFlops() const;
    int GetPrice() const;
    int GetPStateId() const;

    ComputeSpec GetSpec() const;

    bool operator ==(const VMDescription& other) const;
    bool operator !=(const VMDescription& other) const;
    bool operator <(const VMDescription& other) const;
    bool operator >(const VMDescription& other) const;
    bool operator <=(const VMDescription& other) const;
    bool operator >=(const VMDescription& other) const;

private:
    int Id;
    int Cores;
    double Memory;
    double Flops;
    int Price;
    int PStateId;
};
