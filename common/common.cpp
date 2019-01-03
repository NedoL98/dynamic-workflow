#include "common.h"

double ParseNumber(const std::string& size, const std::vector<std::string>& suffixes) {
    try {
        std::size_t suffixStart;
        double sizePrefix = std::stod(size, &suffixStart);
        auto itSizeSuffix = std::find(suffixes.begin(), suffixes.end(), size.substr(suffixStart));
        if (itSizeSuffix == suffixes.end()) {
            throw std::invalid_argument("Size suffix is incorrect!");
        }
        return sizePrefix * pow(Base, itSizeSuffix - suffixes.begin());
    } catch (std::exception& e) {
        throw;
    }
}
