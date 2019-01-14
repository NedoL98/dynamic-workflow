#pragma once

#include <cmath>
#include <simgrid/s4u.hpp>
#include <string>
#include <vector>

static constexpr double Base = 1000;
static const std::vector<std::string> SizeSuffixes = {"B", "KB", "MB", "GB", "TB"};
static const std::vector<std::string> PerformanceSuffixes = {"F", "KF", "MF", "GF", "TF"};

static constexpr int DefaultCores = 1;
static constexpr int DefaultMemory = 0;

double ParseNumber(const std::string& size, const std::vector<std::string>& suffixes);

void TransformHostsProps();