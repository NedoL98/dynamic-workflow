#pragma once
#include "argument_parser.h"
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
    VMList AvailableVMs;
    CloudPlatform Platform;
    AbstractScheduler* Scheduler;
    Schedule Assignments;
    Workflow::Graph TaskGraph;
    std::vector<simgrid::s4u::ActorPtr> Actors;

    struct CollbackData {
        CloudSimulator* Simulator;
        int TaskId;
    };

    static void MainLoop(CloudSimulator* s);
    void DoMainLoop();
    static int RefreshAfterTask(int, void* s);
    void DoRefreshAfterTask(int taskId);
    void CheckReadyJobs();

public:
    CloudSimulator(const std::string& platformConf, 
                   const std::string& workflowConf, 
                   const std::string& VMListConf,
                   AbstractScheduler* scheduler,
                   cxxopts::ParseResult& parseResult):
        AvailableVMs(VMListConf),
        Platform(platformConf, AvailableVMs.MaxMemory()),
        Scheduler(scheduler),
        Assignments(),
        TaskGraph(workflowConf, parseResult)
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
