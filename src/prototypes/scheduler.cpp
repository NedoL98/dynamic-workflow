#include "prototypes/scheduler.h"

AbstractScheduler::Actions StaticScheduler::OnJobComplete(int /* jobId */) {
    return AbstractScheduler::Actions();
}
AbstractScheduler::Actions StaticScheduler::OnJobFail(int /* jobId */) {
    return AbstractScheduler::Actions();
}
