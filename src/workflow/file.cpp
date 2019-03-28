#include <workflow/file.h>

int GetUniqueFileId() {
    static int counter = 0;
    return counter++;
}

namespace Workflow {
    FileDescription::FileDescription(const std::string& name, size_t size):
        Name(name),
        Size(size) {
        Id = GetUniqueFileId();
    }
}

