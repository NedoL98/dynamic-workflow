#include "platform/platform.h"
#include "common/common.h"
#include <simgrid/s4u.hpp>
#include <string>

AbstractPlatform::AbstractPlatform(const std::string& platformConfig) {
    simgrid::s4u::Engine *e = simgrid::s4u::Engine::get_instance();
    e->load_platform(platformConfig);
    TransformHostsProps();
    std::vector<simgrid::s4u::Host *> hosts = e->get_all_hosts();
    for (auto h : hosts) {
        ComputeSpec spec = {std::stoll(h->get_property("memory")),
                         std::stoll(h->get_property("speed")),
                         std::stoll(h->get_property("cores"))};
        HostsList.emplace_back(h, spec);
    }
}

