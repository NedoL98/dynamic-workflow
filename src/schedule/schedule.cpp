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
    PlannedTasks.push(item);
}

bool HostSchedule::HasItem() const {
    return PlannedTasks.size();
}

ScheduleItem HostSchedule::GetItem() const {
    assert(!PlannedTasks.empty());
    return PlannedTasks.front();    
}

ScheduleItem HostSchedule::PopItem() {
    assert(!PlannedTasks.empty());
    ScheduleItem result = PlannedTasks.front();
    PlannedTasks.pop();    
    return std::move(result);
}

bool HostSchedule::IsEmpty() const {
    return PlannedTasks.empty();
}


Schedule::Schedule()
    {}

void Schedule::AddItem(int host, const ScheduleItem& item) {
    TaskIdToHostId[item.GetTaskId()] = host;
    TimeTable[host].AddItem(item);
}

bool Schedule::HasItem(int host) {
    return !TimeTable[host].IsEmpty();
}

ScheduleItem Schedule::GetItem(int host) {
    return TimeTable[host].GetItem();
}

ScheduleItem Schedule::PopItem(int host) {
    int taskId = GetItem(host).GetTaskId();
    return TimeTable[host].PopItem();
}

int Schedule::GetHostByTask(int taskId) const {
    auto assignment = TaskIdToHostId.find(taskId);
    if (assignment != TaskIdToHostId.end()) {
        return assignment->second;
    }
    return -1;
}
