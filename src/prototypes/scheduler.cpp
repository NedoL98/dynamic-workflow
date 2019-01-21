#include "prototypes/scheduler.h"

Schedule AbstractScheduler::GetSchedule() const {
    return schedule;
}


void StaticScheduler::OnJobComplete(int /* jobId */) {
}
void StaticScheduler::OnJobFail(int /* jobId */) {
}
