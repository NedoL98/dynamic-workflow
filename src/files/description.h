#pragma once
#include <string>
#include <set>
enum class EFileState {
    NOT_STARTED,
    READY,
    TRANSFERING,
    DONE
};

struct FileDescription {

    struct FileIterator : std::set<FileDescription*>::const_iterator {
        FileIterator(std::set<FileDescription*>::const_iterator me, std::set<FileDescription*>::const_iterator end)
            : std::set<FileDescription*>::const_iterator(me)
            , End(end)
            {}

        operator bool() {
            return *this != End;
        }

        std::set<FileDescription*>::const_iterator End;
    };
    std::string Name;
    long long Size;
    int Author;
    std::set<int> Receivers;
    EFileState State;
    int Id;

    FileDescription(const std::string& name, long long size)
        : Name(name)
        , Size(size)
        , Author(-1)
        , Receivers()
        , State(EFileState::NOT_STARTED) {
        static int counter = 0;
        Id = counter++;
    }
    bool operator<(const FileDescription& other) const {
        return Id < other.Id;
    }
    bool operator=(const FileDescription& other) const {
        return Id == other.Id;
    }
};

