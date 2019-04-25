#pragma once
#include "argument_parser.h"
#include "prototypes/interface.h"
#include "prototypes/scheduler.h"
#include "platform/platform.h"
#include "schedule/schedule_action.h"
#include "workflow/abstract_graph.h"
#include "vm_list.h"
#include <vector>
#include <map>
#include <string>
#include <memory>

class AbstractSimulator : public SimulatorInterface {
protected:
    AbstractScheduler* Scheduler;
    Workflow::AbstractGraph* TaskGraph;
    Schedule Assignments;
    AbstractPlatform* Platform;
    AbstractSimulator(AbstractScheduler* s, Workflow::AbstractGraph* g, AbstractPlatform* p)
        : Scheduler(s)
        , TaskGraph(g)
        , Assignments()
        , Platform(p)
    {
    }

public:
    void RegisterScheduler(AbstractScheduler* s) {
        Scheduler = s;
    }
    
    void ProcessAction(std::shared_ptr<AbstractAction> a) {
        a->MakeAction(*this);
    }
    
    virtual void SendEvent(AbstractEvent *event) {
        if ((TaskFinishedEvent *)event) {
            Scheduler->OnTaskComplete(*(TaskFinishedEvent *)event);
        } else if ((ActionCompletedEvent *)event) {
            Scheduler->OnActionComplete(*(ActionCompletedEvent *)event);
        }
    }

    virtual void Run(double timeout) = 0;
};

