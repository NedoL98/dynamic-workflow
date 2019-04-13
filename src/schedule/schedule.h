#pragma once

#include <queue>
#include <map>
using std::queue;
using std::map;


class ScheduleItem {
    int TaskId;
    int Priority;
    int ItemId;

public:
    ScheduleItem();
    ScheduleItem(int i);
    ScheduleItem(int i, int p);
    void SetPriority(int p);
    int GetPriority() const;
    int GetTaskId() const;
    int GetItemId() const;
    bool operator<(const ScheduleItem& other);
};


class HostSchedule {
    queue<ScheduleItem> PlannedTasks;
    
public:
    HostSchedule();
    void AddItem(const ScheduleItem& item);
    ScheduleItem PopItem();
    bool HasItem() const;
    ScheduleItem GetItem() const;
    bool IsEmpty() const; 
};

class Schedule {
    map<int, HostSchedule> TimeTable;
    std::map<int, int> TaskIdToHostId;

public:
    Schedule();
    void AddItem(int host, const ScheduleItem& item);
    ScheduleItem GetItem(int host);
    bool HasItem(int host);
    ScheduleItem PopItem(int host);
    int GetHostByTask(int taskId) const;
};
