#pragma once
#include <string>
#include <set>
enum EFileState {
    NOT_STARTED,
    READY,
    TRANSFERING,
    DONE
};

struct FileDescription {
    struct FileIterator : std::set<FileDescription>::const_iterator {
        FileIterator(std::set<FileDescription>::const_iterator me, std::set<FileDescription>::const_iterator end)
            : std::set<FileDescription>::const_iterator(me)
            , End(end)
            {}

        operator bool() {
            return End != *this;
        }
        std::set<FileDescription>::const_iterator End;

    };
    std::string Name;
    long long Size;
    int Author, Receiver;
    EFileState State;
    int Id;

    FileDescription(const std::string& name, long long size)
        : Name(name)
        , Size(size)
        , Author(-1)
        , Receiver(-1)
        , State(EFileState::NOT_STARTED) {
        static int counter = 0;
        Id = counter++;
    }
    bool operator<(const FileDescription& other) const {
        return Id < other.Id;
    }
};

