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

bool Host::CreateVM(const ComputeSpec& c, int CustomId) {
    if (VirtualMachines.count(CustomId)) {
        return false;
    }
    /*
    if (AvailiableCores < c.Cores || AvailiableMemory < c.Memory) {
        return false;
    }
    */
    simgrid::s4u::VirtualMachine* vm = new simgrid::s4u::VirtualMachine(std::to_string(Id) + std::to_string(VirtualMachines.size()) + "_VM", Unit, c.Cores, c.Memory);
    vm->set_property("VM_ID", std::to_string(CustomId));
    AvailiableCores -= c.Cores;
    AvailiableMemory -= c.Memory;
    VirtualMachines[CustomId] = vm;
    vm->start();
    return true;
}
