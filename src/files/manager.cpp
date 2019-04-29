#include <vector>
#include <map>
#include <files/description.h>
#include <files/manager.h>
#include <common/common.h>
#include <simgrid/s4u.hpp>
#include <workflow/graph.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(files_manager, "file manager log");
using std::map;
using std::vector;
using std::set;
using std::string;

    // returns: id of new file
int Manager::TryAddFile(const YAML::Node& description) {
    xbt_assert(description["name"], "File name is not specified!");
    string name = description["name"].as<string>(); 
    if (Files.count(name) > 0) {
        XBT_WARN("name is not unique! Previous file will be deleted!");
    }
    xbt_assert(description["size"], "Size is not specified!");
    map<string, FileDescription>::iterator insertionResult;
    try {
        insertionResult = Files.insert({name, FileDescription(name, ParseSize(description["size"].as<string>(), SizeSuffixes))}).first;
    } catch (std::exception& e) {
        XBT_ERROR("Can't parse file size: %s", e.what());
        XBT_DEBUG("File size will be set to 0");
        insertionResult = Files.insert({name, FileDescription(name, 0)}).first;
    }
    FileIdMapping.insert({insertionResult->second.Id, &insertionResult->second});
    return insertionResult->second.Id;
}

const FileDescription& Manager::GetFileById(int id) const {
    auto result = FileIdMapping.find(id);
    xbt_assert(result != FileIdMapping.end(), "no file with such id");
    return *result->second;
}

const FileDescription& Manager::GetFileByName(const string& s) const {
    auto result = Files.find(s);
    xbt_assert(result != Files.end(), "no file with such name: %s\n", s.c_str());
    return result->second;
}

void Manager::RegisterTaskFiles(int taskId) {
    auto task = Owner->GetTask(taskId);
    if (!TaskDerivatives.count(taskId)) {
        TaskDerivatives[taskId] = set<int>(task.GetOutputs().cbegin(), task.GetOutputs().cend());
        TaskDependencies[taskId] = set<int>(task.GetInputs().cbegin(), task.GetInputs().cend());
    }
}

void Manager::SetAuthor(int fileId, int taskId) {
    FileIdMapping.find(fileId)->second->Author = taskId;
}

void Manager::AddReceiver(int fileId, int taskId) {
    XBT_INFO("taskId %d", taskId);
    FileIdMapping.find(fileId)->second->Receivers.insert(taskId);
}

void Manager::SetInputs(const vector<int>& data) {
    WorkflowInputs = set<int>(data.begin(), data.end());
}

void Manager::SetOutputs(const vector<int>& data) {
    WorkflowOutputs = set<int>(data.begin(), data.end());
}

void Manager::FinishTask(int taskId) {
    XBT_INFO("Try to transfer smth.. after finishing %d", taskId);
    auto task = Owner->GetTask(taskId);
    for (int file : task.GetOutputs()) {
        auto fileDesc = FileIdMapping.find(file)->second;
        if (fileDesc->Author == -1) {
            FileIdMapping.find(file)->second->State = EFileState::DONE;
        }
        if (WorkflowOutputs.count(fileDesc->Id)) {
            OutputProduced++;
        }
        for (int receiver : fileDesc->Receivers) {
            if (receiver != -1) {
                if (TaskToHost.count(receiver) && fileDesc->State == EFileState::NOT_STARTED) {
                    fileDesc->State = EFileState::READY;
                    ReadyFiles.insert(fileDesc);
                }
            } 
        }
    }
}

FileDescription::FileIterator Manager::GetReadyFilesIterator() const {
    XBT_INFO("%d files", ReadyFiles.size());
    return FileDescription::FileIterator(ReadyFiles.cbegin(), ReadyFiles.cend());
}

void Manager::StartTransfer(int fileId) {
    ReadyFiles.erase(FileIdMapping[fileId]);
    FileIdMapping.find(fileId)->second->State = EFileState::TRANSFERING;
}
void Manager::FinishTransfer(TransferSpec spec) {
}

void Manager::AssignTask(int taskId, int hostId) {
    XBT_INFO("Try to transfer smth.. after scheduling %d", taskId);
    TaskToHost[taskId] = hostId;
    auto task = Owner->GetTask(taskId);
    for (int file : task.GetInputs()) {
        auto fileDesc = FileIdMapping.find(file)->second;
        int sender = fileDesc->Author;
        if (sender != -1 && Owner->GetTask(sender).GetState() == EState::DONE && fileDesc->State == EFileState::NOT_STARTED) {
            fileDesc->State = EFileState::READY;
            ReadyFiles.insert(fileDesc);
        }
    }
}

bool Manager::IsFinished() const {
    XBT_INFO("%d ?=? %d", OutputProduced, static_cast<int>(WorkflowOutputs.size()));
    return OutputProduced == static_cast<int>(WorkflowOutputs.size());
}
