#pragma once

#include "spec.h"

#include <vector>
#include <memory>

#include <simgrid/s4u.hpp>
#include <simgrid/s4u/VirtualMachine.hpp>

class Host {
public:
    typedef std::vector<simgrid::s4u::VirtualMachine* > VMs;
    typedef std::vector<simgrid::s4u::ActorPtr> ActorPointers;

private:
    VMs VirtualMachines;
    ActorPointers Actors;
    ComputeSpec Spec;
    simgrid::s4u::Host *Unit;
    long long AvailiableMemory;
    int AvailiableCores;
    int Id;

public:
    Host(simgrid::s4u::Host *host, const ComputeSpec& c);
    
    int CreateVM(const ComputeSpec& c);
};
