#include "platform/host.h"

static int GetUniqueHostId() {
    static int id = 0;
    return id++;
}

Host::Host(simgrid::s4u::Host *host, const ComputeSpec& c):
    Spec(c),
    Unit(host),
    AvailiableMemory(c.Memory),
    AvailiableCores(c.Cores) {
    Id = GetUniqueHostId();
}

int Host::CreateVM(const ComputeSpec& c) {
    if (AvailiableCores < c.Cores || AvailiableMemory < c.Memory) {
        return -1;
    }
    simgrid::s4u::VirtualMachine* vm = new simgrid::s4u::VirtualMachine(std::to_string(Id) + std::to_string(VirtualMachines.size()) + "_VM", Unit, c.Cores, c.Memory);
    AvailiableCores -= c.Cores;
    AvailiableMemory -= c.Memory;
    VirtualMachines.emplace_back(vm);
    return VirtualMachines.size() - 1;
}
