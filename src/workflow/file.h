#pragma once
#include <string>
#include <map>

namespace Workflow {
    struct FileDescription {
        std::string Name;
        size_t Size;
        int Id;

        FileDescription(const std::string& name, size_t size);
    };
    typedef std::map<std::string, FileDescription> FileRegistry;
}
