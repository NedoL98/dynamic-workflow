#pragma once
#include "prototypes/interface.h"
#include "prototypes/scheduler.h"
#include "platform/platform.h"
#include "schedule/schedule_action.h"
#include "workflow/graph.h"
#include "vm_list.h"
#include <vector>
#include <map>
#include <string>
#include <memory>

class CloudSimulator : public SimulatorInterface {
    VMList AvailiableVMs;
    CloudPlatform Platform;
    AbstractScheduler* Scheduler;
    Schedule Assignments;
    Workflow::Graph TaskGraph;

public:
    CloudSimulator(const std::string& platformConf, 
                   const std::string& workflowConf, 
                   const std::string& VMListConf,
                   AbstractScheduler* scheduler):
        AvailiableVMs(VMListConf),
        Platform(platformConf),
        Scheduler(scheduler),
        Assignments(),
        TaskGraph(workflowConf)
        {}
    
    void RegisterScheduler(AbstractScheduler* s) {
        Scheduler = s;
    }
    
    void ProcessAction(std::shared_ptr<AbstractAction> a) {
        a->MakeAction(*this);
    }
    
    void SendEvent(AbstractEvent *event) {
        if ((TaskFinishedEvent *)event) {
            Scheduler->OnTaskComplete(*(TaskFinishedEvent *)event);
        } else if ((ActionCompletedEvent *)event) {
            Scheduler->OnActionComplete(*(ActionCompletedEvent *)event);
        }
    }

    void Run(double timeout=0);   
    virtual bool RegisterVirtualMachine(const VMDescription &stats, int customId) override;
    virtual bool AssignTask(int hostId, const ScheduleItem &item) override;
    virtual bool CancelTask(int hostId, const ScheduleItem &item) override;
    virtual bool ResetSchedule(const Schedule& s) override;
    friend class View::Viewer;
};
