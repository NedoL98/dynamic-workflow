#include "common.h"

#include <string>
#include <unordered_map>
#include <vector>

using std::string;
using std::unordered_map;
using std::vector;

double ParseSize(const string& size, const vector<string>& suffixes) {
    size_t suffixStart;
    double sizePrefix = std::stod(size, &suffixStart);
    if (suffixStart == size.size()) {
        return sizePrefix;
    } else {
        auto itSizeSuffix = std::find(suffixes.begin(), suffixes.end(), size.substr(suffixStart));
        if (itSizeSuffix == suffixes.end()) {
            throw std::invalid_argument("Size suffix is incorrect!");
        }
        return sizePrefix * pow(BASE, itSizeSuffix - suffixes.begin());
    }
}

void TransformHostsProps() {
    vector<simgrid::s4u::Host*> hosts = simgrid::s4u::Engine::get_instance()->get_all_hosts();
    for (simgrid::s4u::Host* host: hosts) {
        host->set_property("cores", std::to_string(host->get_core_count()));

        unordered_map<string, string> hostProps = *(host->get_properties());
        if (!hostProps.count("memory")) {
            throw std::invalid_argument("No memory limit for host" + host->get_name() + "is specified");
        }
        host->set_property("memory", std::to_string(ParseSize(hostProps["memory"], SizeSuffixes)));
    }
}
