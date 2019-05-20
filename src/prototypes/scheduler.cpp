#include "prototypes/scheduler.h"

AbstractScheduler::Actions StaticScheduler::OnTaskComplete(std::shared_ptr<TaskFinishedEvent> /*e*/) {
    return AbstractScheduler::Actions();
}
AbstractScheduler::Actions StaticScheduler::OnTaskFail(std::shared_ptr<TaskFinishedEvent> /*e*/) {
    return AbstractScheduler::Actions();
}

AbstractScheduler::Actions StaticScheduler::OnActionComplete(std::shared_ptr<ActionCompletedEvent>/*e*/) {
    return AbstractScheduler::Actions();
}
AbstractScheduler::Actions StaticScheduler::OnTransferStarted(std::shared_ptr<TransferStartedEvent> /*e*/) {
    return AbstractScheduler::Actions();
}
AbstractScheduler::Actions StaticScheduler::OnTransferFinished(std::shared_ptr<TransferFinishedEvent> /*e*/) {
    return AbstractScheduler::Actions();
}
AbstractScheduler::Actions StaticScheduler::OnTaskStarted(std::shared_ptr<TaskStartedEvent> /*e*/) {
    return AbstractScheduler::Actions();
}
