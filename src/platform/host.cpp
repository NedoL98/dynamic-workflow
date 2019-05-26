#include "platform/host.h"

static int GetUniqueHostId() {
    static int id = 0;
    return id++;
}

Host::Host(simgrid::s4u::Host *host, const ComputeSpec& c):
    Spec(c),
    Unit(host),
    AvailableMemory(c.Memory),
    AvailableCores(c.Cores) {
    Id = GetUniqueHostId();
}

bool Host::CreateVM(const VMDescription& c, int CustomId) {
    if (VirtualMachines.count(CustomId)) {
        return false;
    }
    if (AvailableCores < c.GetCores() || AvailableMemory < c.GetMemory()) {
        return false;
    }
    Unit->set_pstate(c.GetPStateId());
    simgrid::s4u::VirtualMachine* vm = new simgrid::s4u::VirtualMachine(std::to_string(Id) + "_VM", Unit, c.GetCores(), c.GetMemory());
    vm->set_property("VM_ID", std::to_string(CustomId));
    vm->set_property("vm_cost", std::to_string(c.GetPStateId()));
    AvailableCores -= c.GetCores();
    AvailableMemory -= c.GetMemory();
    VirtualMachines[CustomId] = vm;
    return true;
}
