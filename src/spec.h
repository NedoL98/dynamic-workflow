#pragma once
struct ComputeSpec {
    long long Memory, Speed;
    int Cores;
    bool operator ==(const ComputeSpec& other) const {
        return Speed == other.Speed && Cores == other.Cores;
    }
};

struct TaskSpec {
    long long Memory, Cost;
    int Cores;
};

enum EState {
    NotScheduled,
    Scheduled,
    Ready,
    Running,
    Done
};

class VMDescription {
public:
    VMDescription() = default;
    VMDescription(int id, int cores, long long memory,
                    long long flops, int price, int pStateId);

    int GetId() const;
    int GetCores() const;
    long long GetMemory() const;
    long long GetFlops() const;
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
    long long Memory;
    long long Flops;
    int Price;
    int PStateId;
};
