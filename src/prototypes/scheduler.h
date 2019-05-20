#pragma once
#include "schedule/schedule.h"
#include "schedule/schedule_action.h"
#include "events/event.h"
#include <vector>
#include <memory>

namespace View {
    class Viewer;
}

class AbstractScheduler {
protected:
    std::shared_ptr<View::Viewer> viewer;
public:
    typedef std::vector<std::shared_ptr<AbstractAction> > Actions;
    virtual Actions PrepareForRun(View::Viewer& v) = 0;
    virtual Actions OnTransferStarted(std::shared_ptr<TransferStartedEvent> e) = 0;
    virtual Actions OnTransferFinished(std::shared_ptr<TransferFinishedEvent> e) = 0;
    virtual Actions OnTaskStarted(std::shared_ptr<TaskStartedEvent> e) = 0;
    virtual Actions OnTaskComplete(std::shared_ptr<TaskFinishedEvent> e) = 0;
    virtual Actions OnTaskFail(std::shared_ptr<TaskFinishedEvent> e) = 0;
    virtual Actions OnActionComplete(std::shared_ptr<ActionCompletedEvent> e) = 0;
};

class StaticScheduler : public AbstractScheduler {

public:
    virtual Actions OnTransferStarted(std::shared_ptr<TransferStartedEvent> e) override final;
    virtual Actions OnTransferFinished(std::shared_ptr<TransferFinishedEvent> e) override final;
    virtual Actions OnTaskStarted(std::shared_ptr<TaskStartedEvent> e) override final;
    virtual Actions OnTaskComplete(std::shared_ptr<TaskFinishedEvent> e) override final;
    virtual Actions OnTaskFail(std::shared_ptr<TaskFinishedEvent> e) override final;
    virtual Actions OnActionComplete(std::shared_ptr<ActionCompletedEvent> e) override final;
};
