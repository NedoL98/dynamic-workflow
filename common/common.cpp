#include "common.h"

#include <iostream>

double ParseNumber(const std::string& size, const std::vector<std::string>& suffixes) {
    std::size_t suffixStart;
    double sizePrefix = std::stod(size, &suffixStart);
    if (suffixStart == size.size()) {
        return sizePrefix;
    } else {
        auto itSizeSuffix = std::find(suffixes.begin(), suffixes.end(), size.substr(suffixStart));
        if (itSizeSuffix == suffixes.end()) {
            throw std::invalid_argument("Size suffix is incorrect!");
        }
        return sizePrefix * pow(Base, itSizeSuffix - suffixes.begin());
    }
}

void TransformHostsProps() {
    std::vector<simgrid::s4u::Host*> hosts = simgrid::s4u::Engine::get_instance()->get_all_hosts();
    for (simgrid::s4u::Host* host: hosts) {
        host->set_property("cores", std::to_string(host->get_core_count()));

        std::unordered_map<std::string, std::string> hostProps = *(host->get_properties());
        if (!hostProps.count("memory")) {
            throw std::invalid_argument("No memory limit for host" + host->get_name() + "is specified");
        }
        host->set_property("memory", std::to_string(ParseNumber(hostProps["memory"], SizeSuffixes)));
    }
}
