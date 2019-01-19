#pragma once

#include <queue>
#include <vector>
using std::queue;


class ScheduleItem {
    int taskId;

public:
    ScheduleItem();
    ScheduleItem(int i);
    ScheduleItem(int i, int p);
    void SetPriority(int p);
    int GetPriority() const;
    int GetId() const;
    bool operator<(const ScheduleItem& other);
};

class HostSchedule {
    queue<ScheduleItem> plannedJobs;
    
public:
    HostSchedule();
    void AddItem(const ScheduleItem& item);
    ScheduleItem PopItem();
    ScheduleItem GetItem() const;
    bool IsEmpty() const; 
};

class Schedule {
    vector<HostSchedule> timeTable;

public:
    Schedule(int hostsCount);
    void AddItem(int host, const ScheduleItem& item);
    void GetItem(int host);
};
