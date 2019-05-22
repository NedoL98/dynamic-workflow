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
#include <fstream>

class AbstractSimulator : public SimulatorInterface {
protected:
    std::ofstream log;
    simgrid::s4u::MutexPtr logMutex;
    AbstractScheduler* Scheduler;
    Workflow::AbstractGraph* TaskGraph;
    Schedule Assignments;
    AbstractPlatform* Platform;
    AbstractSimulator(AbstractScheduler* s, Workflow::AbstractGraph* g, AbstractPlatform* p, const std::string& logPath)
        : log(logPath)
        , logMutex(simgrid::s4u::Mutex::create())
        , Scheduler(s)
        , TaskGraph(g)
        , Assignments()
        , Platform(p)
    {
    }
    template <typename T, typename... Args>
    std::shared_ptr<T> GenerateEvent(EventStatus status, Args&... args) {
        auto ret = std::make_shared<T>(T(status, args...));
        logMutex->lock();
        ret->Dump(log);
        logMutex->unlock();
        return ret;
    }

public:
    void RegisterScheduler(AbstractScheduler* s) {
        Scheduler = s;
    }
    
    void ProcessAction(std::shared_ptr<AbstractAction> a) {

        a->MakeAction(*this);
        Scheduler->OnActionComplete(GenerateEvent<ActionCompletedEvent>(EventStatus::Succeed, a));
    }
    
    virtual void Run(double timeout) = 0;
};

