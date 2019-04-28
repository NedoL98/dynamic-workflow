#pragma once
#include "argument_parser.h"
#include "prototypes/interface.h"
#include "prototypes/scheduler.h"
#include "platform/cloud_platform.h"
#include "schedule/schedule_action.h"
#include "workflow/graph.h"
#include "vm_list.h"
#include <vector>
#include <map>
#include <string>
#include <memory>
#include "prototypes/simulator.h"

class CloudSimulator : public AbstractSimulator {
    VMList AvailableVMs;
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
    void CheckReadyFiles();

public:
    CloudSimulator(const std::string& platformConf, 
                   const std::string& workflowConf, 
                   const std::string& VMListConf,
                   AbstractScheduler* scheduler,
                   cxxopts::ParseResult& parseResult)
        : AbstractSimulator(scheduler,
                            new Workflow::Graph(workflowConf, parseResult),
                            new CloudPlatform(platformConf, VMList(VMListConf)))
        , AvailableVMs(VMListConf)
        {}
    
    virtual bool RegisterVirtualMachine(const VMDescription &stats, int customId) override;
    virtual bool AssignTask(int hostId, const ScheduleItem &item) override;
    virtual bool CancelTask(int hostId, const ScheduleItem &item) override;
    virtual bool ResetSchedule(const Schedule& s) override;
    virtual void Run(double timeout = 0) override;
    friend class View::Viewer;
};
