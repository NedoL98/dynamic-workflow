#include <cassert>
#include <schedule/schedule.h>

int GetUniqueScheduleItemId() {
    static int counter = 0;
    return counter++;
}

ScheduleItem::ScheduleItem():
    TaskId(0),
    Priority(-1) {
    ItemId = GetUniqueScheduleItemId();
}

ScheduleItem::ScheduleItem(int id):
    TaskId(id),
    Priority(-1) {
    ItemId = GetUniqueScheduleItemId();
}

ScheduleItem::ScheduleItem(int id, int p):
    TaskId(id),
    Priority(p) {
    ItemId = GetUniqueScheduleItemId();
}

void ScheduleItem::SetPriority(int p) {
    Priority = p;
}

int ScheduleItem::GetPriority() const {
    return Priority;
}


int ScheduleItem::GetItemId() const {
    return ItemId;
}

int ScheduleItem::GetTaskId() const {
    return TaskId;
}

bool ScheduleItem::operator<(const ScheduleItem& other) {
    if (Priority == other.Priority) {
        return TaskId < other.TaskId;
    }
    return Priority < other.Priority;
}

HostSchedule::HostSchedule() {}
void HostSchedule::AddItem(const ScheduleItem& item) {
    PlannedJobs.push(item);
}

ScheduleItem HostSchedule::GetItem() const {
    assert(PlannedJobs.empty());
    return PlannedJobs.front();    
}

ScheduleItem HostSchedule::PopItem() {
    assert(PlannedJobs.empty());
    ScheduleItem result = PlannedJobs.front();
    PlannedJobs.pop();    
    return std::move(result);
}

bool HostSchedule::IsEmpty() const {
    return PlannedJobs.empty();
}


Schedule::Schedule(int hostsCount):
    TimeTable(hostsCount)
    {}

void Schedule::AddItem(int host, const ScheduleItem& item) {
    TimeTable[host].AddItem(item);
}

ScheduleItem Schedule::GetItem(int host) {
    return TimeTable[host].GetItem();
}

ScheduleItem Schedule::PopItem(int host) {
    return TimeTable[host].PopItem();
}
