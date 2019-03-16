#pragma once

#include "spec.h"

#include <vector>
#include <memory>

#include <simgrid/s4u.hpp>
#include <simgrid/s4u/VirtualMachine.hpp>

struct Host {
    typedef std::map<int, simgrid::s4u::VirtualMachine* > VMs;
    typedef std::map<int, simgrid::s4u::ActorPtr> ActorPointers;

    VMs VirtualMachines;
    ActorPointers Actors;
    ComputeSpec Spec;
    simgrid::s4u::Host *Unit;
    long long AvailiableMemory;
    int AvailiableCores;
    int Id;

    Host(simgrid::s4u::Host *host, const ComputeSpec& c);
    
    bool CreateVM(const ComputeSpec& c, int CustomId);
};
