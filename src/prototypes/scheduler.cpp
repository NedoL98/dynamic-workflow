#include "prototypes/scheduler.h"

AbstractScheduler::Actions StaticScheduler::OnJobComplete(const JobFinishedEvent& /*e*/) {
    return AbstractScheduler::Actions();
}
AbstractScheduler::Actions StaticScheduler::OnJobFail(const JobFinishedEvent& /*e*/) {
    return AbstractScheduler::Actions();
}

AbstractScheduler::Actions StaticScheduler::OnActionComplete(const ActionCompletedEvent& /*e*/) {
    return AbstractScheduler::Actions();
}
