#pragma once

#include <cmath>
#include <simgrid/s4u.hpp>
#include <string>
#include <vector>

// XBT_LOG_NEW_DEFAULT_CATEGORY(common, "Common log");
// XBT_LOG_DEFAULT_CATEGORY(common);

static constexpr double Base = 1000;
static const std::vector<std::string> SizeSuffixes = {"B", "KB", "MB", "GB", "TB"};
static const std::vector<std::string> PerformanceSuffixes = {"F", "KF", "MF", "GF", "TF"};

static double ParseNumber(const std::string& size, const std::vector<std::string>& suffixes) {
    try {
        std::size_t suffixStart;
        double sizePrefix = std::stod(size, &suffixStart);
        auto itSizeSuffix = std::find(suffixes.begin(), suffixes.end(), size.substr(suffixStart));
        if (itSizeSuffix == suffixes.end()) {
            throw std::invalid_argument("Size suffix is not found!");
        }
        return sizePrefix * pow(Base, itSizeSuffix - suffixes.begin());
    } catch (std::exception& e) {
        // XBT_INFO(e.what());
        // XBT_WARN("Can't parse size!");
        return 0;
    }
}