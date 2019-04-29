#pragma once
#include <yaml-cpp/yaml.h>

struct TransferSpec {
    long long Size;
    int Sender, Receiver;
    int FileId;
};

struct ComputeSpec {
    long long Memory, Speed;
    int Cores;
    bool operator ==(const ComputeSpec& other) const {
        return Speed == other.Speed && Cores == other.Cores;
    }
};

class TaskSpec {
friend class Task;
public:
    TaskSpec() = default;
    TaskSpec(const YAML::Node& taskDescription);

    long long GetMemory() const;
    long long GetSize() const;
    int GetCores() const;

private:
    long long Memory, Size;
    int Cores;
};

enum class EState {
    NOT_SCHEDULED,
    SCHEDULED,
    READY,
    RUNNING,
    DONE
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
