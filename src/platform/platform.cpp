#include "platform/platform.h"
#include "common/common.h"
#include <simgrid/s4u.hpp>
#include <string>
#include <vector>
using std::vector;

AbstractPlatform::AbstractPlatform(const std::string& platformConfig, long long hostMaxMemory) {
    simgrid::s4u::Engine *e = simgrid::s4u::Engine::get_instance();
    e->load_platform(platformConfig);
    TransformHostsProps();
    std::vector<simgrid::s4u::Host *> hosts = e->get_all_hosts();
    for (auto h : hosts) {
        ComputeSpec spec = {hostMaxMemory,
                            (long long)h->get_speed(),
                            h->get_core_count()};
        HostsList.emplace_back(h, spec);
    }
}
vector<int> AbstractPlatform::GetVMIds() const {
    vector<int> ids;
    for (auto it : VirtualMachines) {
        ids.push_back(it.first);
    }
    return ids;
}
