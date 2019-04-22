#pragma once
#include <string>

struct FileDescription {
    std::string Name;
    long long Size;
    int Author, Receiver;
    int Id;

    FileDescription(const std::string& name, long long size):
        Name(name),
        Size(size),
        Author(-1),
        Receiver(-1) {
        static int counter = 0;
        Id = counter++;
    }
};

