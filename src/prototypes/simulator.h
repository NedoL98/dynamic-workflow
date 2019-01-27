#pragma once

#include "spec.h"
#include "schedule/schedule.h"
#include <vector>
#include <map>
#include <string>

class SimulatorInterface {
public:
    SimulatorInterface(const SimulatorInterface& other) = delete;
    SimulatorInterface& operator=(const SimulatorInterface& other) = delete;

    virtual bool RegisterVirtualMachine(const ComputeSpec &stats) = 0;
    virtual bool AssignJob(int hostId, const ScheduleItem &item) = 0;
    virtual bool CancelJob(int hostId, const ScheduleItem &item) = 0;
    virtual bool ResetSchedule(const Schedule &s) = 0;
};

class AbstractSimulator : public SimulatorInterface {
    
};
