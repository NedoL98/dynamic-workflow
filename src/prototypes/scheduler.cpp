#include "prototypes/scheduler.h"

AbstractScheduler::Actions StaticScheduler::OnTaskComplete(const TaskFinishedEvent& /*e*/) {
    return AbstractScheduler::Actions();
}
AbstractScheduler::Actions StaticScheduler::OnTaskFail(const TaskFinishedEvent& /*e*/) {
    return AbstractScheduler::Actions();
}

AbstractScheduler::Actions StaticScheduler::OnActionComplete(const ActionCompletedEvent& /*e*/) {
    return AbstractScheduler::Actions();
}
