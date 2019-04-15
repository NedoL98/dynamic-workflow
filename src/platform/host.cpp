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

bool Host::CreateVM(const VMDescription& c, int CustomId) {
    if (VirtualMachines.count(CustomId)) {
        return false;
    }
    /* FIXME
    if (AvailiableCores < c.Cores || AvailiableMemory < c.Memory) {
        return false;
    }
    */
    Unit->set_pstate(c.GetPStateId());
    simgrid::s4u::VirtualMachine* vm = new simgrid::s4u::VirtualMachine(std::to_string(Id) + "_VM", Unit, c.GetCores()); // FIXME add memory
    vm->set_property("VM_ID", std::to_string(CustomId));
    AvailiableCores -= c.GetCores();
    AvailiableMemory -= c.GetMemory();
    VirtualMachines[CustomId] = vm;
    return true;
}
