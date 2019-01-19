#include <schedule/schedule.h>

ScheduleItem::ScheduleItem():
    taskId(0),
    priority(-1)
{}

ScheduleItem::ScheduleItem(int id):
    taskId(id),
    priority(-1)
{}

ScheduleItem::ScheduleItem(int id, int p):
    taskId(id),
    priority(p)
{}

void ScheduleItem::SetPriority(int p) {
    priority = p;
}

int ScheduleItem::GetPriority() const {
    return priority;
}


int ScheduleItem::GetId() const {
    return taskId;
}

bool ScheduleItem::operator<(const ScheduleItem& other) {
    if (priority == other.priority) {
        return taskId < other.taskId;
    }
    return priority < other.priority;
}

HostSchedule::HostSchedule() {}
void HostSchedule::AddItem(const ScheduleItem& item) {
    plannedJobs.push_back(item);
}

ScheduleItem HostSchedule::GetItem() {
    assert(plannedJobs.empty());
    return plannedJobs.top();    
}

ScheduleItem HostSchedule::popItem() {
    assert(plannedJobs.empty());
    return plannedJobs.pop_front();    
}

bool HostSchedule::IsEmpty() const {
    return plannedJobs.empty();
}


Schedule::Schedule(int hostsCount):
    timeTable(hostsCount)
    {}

void Schedule::AddItem(int host, const ScheduleItem& item) {
    timeTable[host].AddItem(item);
}

void Schedule::GetItem(int host) {
    return timeTable[host].GetItem();
}
