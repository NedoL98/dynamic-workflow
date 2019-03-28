#pragma once
#include <string>

struct FileDescription {
    std::string Name;
    size_t Size;
    int Id;

    FileDescription(const std::string& name, size_t size);
};
