#pragma once
#include "prototypes/interface.h"
#include "prototypes/scheduler.h"
#include "platform/platform.h"
#include "schedule/schedule_action.h"
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

public:
    CloudSimulator(const std::string& platformConf, const std::string& workflowConf, const std::string& VMListConf):
        AvailiableVMs(VMListConf),
        Platform(platformConf),
        Scheduler(nullptr),
        Assignments(0)
        {}
    
    void RegisterScheduler(AbstractScheduler* s) {
        Scheduler = s;
    }
    
    void ProcessAction(std::shared_ptr<AbstractAction> a) {
        a->MakeAction(*this);
    }

    void Run(double timeout=0);   
    friend class View::Viewer;
};
