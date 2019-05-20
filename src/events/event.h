#pragma once
#include <simgrid/s4u.hpp>
#include "schedule/schedule.h"
#include <memory>
#include <string>
#include <iostream>

enum class EventStatus {
    Succeed,
    Failed
};

enum class EventType {
    TaskStarted,
    TaskFinished,
    TransferStarted,
    TransferFinished,
    ActionCompleted
};

namespace {
    std::string ToString(EventType t) {
        if (t == EventType::TaskStarted) {
            return "task_started";
        } else if (t == EventType::TaskFinished) {
            return "task_finished";
        } else if (t == EventType::TransferStarted) {
            return "transfer_started";
        } else if (t == EventType::TransferFinished) {
            return "transfer_finished";
        } else if (t == EventType::ActionCompleted) {
            return "action_completed";
        }
        return "unknown";
    }
};

class AbstractEvent {
    EventStatus Status;

    virtual YAML::Node DoDump() = 0;
public:
    AbstractEvent(EventStatus s):
        Status(s) {
    }
    
    EventStatus GetStatus() const {
        return Status;
    }

    virtual EventType GetType() const = 0;
    void Dump(std::ostream& stream) {
        YAML::Node dumpResult;
        dumpResult["type"] = ToString(GetType());
        dumpResult["time"] = simgrid::s4u::Engine::get_clock();
        auto host = simgrid::s4u::this_actor::get_host();
        if (host) {
            dumpResult["host"] = host->get_cname();
        } else {
            dumpResult["host"] = "";
        }
        dumpResult["event"] = DoDump();      
        stream << dumpResult << "\n" << "\n";
        stream.flush();
    }
};

class TaskStartedEvent : public AbstractEvent {
    int TaskId, HostId;

public:
    TaskStartedEvent(EventStatus s, int taskId, int hostId):
        AbstractEvent(s),
        TaskId(taskId),
        HostId(hostId) {
    }
    int GetTaskId() const {
        return TaskId;
    }

    int GetHostId() const {
        return HostId;
    }
    virtual EventType GetType() const override {
        return EventType::TaskStarted;
    }

    virtual YAML::Node DoDump() override {
        YAML::Node dumpResult;
        dumpResult["task_id"] = TaskId;
        dumpResult["host_id"] = HostId;
        return dumpResult;
    }
};

class TaskFinishedEvent : public AbstractEvent {
    int TaskId, HostId;

public:
    TaskFinishedEvent(EventStatus s, int taskId, int hostId):
        AbstractEvent(s),
        TaskId(taskId),
        HostId(hostId) {
    }
    int GetTaskId() const {
        return TaskId;
    }

    int GetHostId() const {
        return HostId;
    }
    virtual EventType GetType() const override {
        return EventType::TaskFinished;
    }

    virtual YAML::Node DoDump() override {
        YAML::Node dumpResult;
        dumpResult["task_id"] = TaskId;
        dumpResult["host_id"] = HostId;
        return dumpResult;
    }
};

class TransferStartedEvent : public AbstractEvent {
    int FileId, Sender, Receiver;

public:
    TransferStartedEvent(EventStatus s, int fileId, int sender, int receiver):
        AbstractEvent(s),
        FileId(fileId),
        Sender(sender),
        Receiver(receiver) {
    }
    int GetFileId() const {
        return FileId;
    }
    int GetSender() const {
        return Sender;
    }
    int GetReceiver() const {
        return Receiver;
    }
    virtual EventType GetType() const override {
        return EventType::TransferStarted;
    }

    virtual YAML::Node DoDump() override {
        YAML::Node dumpResult;
        dumpResult["file_id"] = FileId;
        dumpResult["sender"] = Sender;
        dumpResult["receiver"] = Receiver;
        return dumpResult;
    }
};

class TransferFinishedEvent : public AbstractEvent {
    int FileId, Sender, Receiver;

public:
    TransferFinishedEvent(EventStatus s, int fileId, int sender, int receiver):
        AbstractEvent(s),
        FileId(fileId),
        Sender(sender),
        Receiver(receiver) {
    }
    int GetFileId() const {
        return FileId;
    }
    int GetSender() const {
        return Sender;
    }
    int GetReceiver() const {
        return Receiver;
    }
    virtual EventType GetType() const override {
        return EventType::TransferFinished;
    }
    virtual YAML::Node DoDump() override {
        YAML::Node dumpResult;
        dumpResult["file_id"] = FileId;
        dumpResult["sender"] = Sender;
        dumpResult["receiver"] = Receiver;
        return dumpResult;
    }
};

class ActionCompletedEvent : public AbstractEvent {
    const std::shared_ptr<AbstractAction> Action;
    
public:
    ActionCompletedEvent(EventStatus s, std::shared_ptr<AbstractAction> action):
        AbstractEvent(s),
        Action(action) {
    }

    const std::shared_ptr<AbstractAction> GetAction() const {
        return Action;
    }
    virtual EventType GetType() const override {
        return EventType::ActionCompleted;
    }
    virtual YAML::Node DoDump() override {
        YAML::Node dumpResult;
        return dumpResult;
    }
};
