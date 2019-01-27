#pragma once

#include <queue>
#include <vector>
using std::queue;
using std::vector;


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
    queue<ScheduleItem> PlannedJobs;
    
public:
    HostSchedule();
    void AddItem(const ScheduleItem& item);
    ScheduleItem PopItem();
    ScheduleItem GetItem() const;
    bool IsEmpty() const; 
};

class Schedule {
    vector<HostSchedule> TimeTable;

public:
    Schedule(int hostsCount);
    void AddItem(int host, const ScheduleItem& item);
    ScheduleItem GetItem(int host);
    ScheduleItem PopItem(int host);
};
