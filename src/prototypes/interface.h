#pragma once
#include "spec.h"
#include "schedule/schedule.h"


class SimulatorInterface {
public:
    SimulatorInterface() = default;
    SimulatorInterface(const SimulatorInterface& other) = delete;
    SimulatorInterface& operator=(const SimulatorInterface& other) = delete;

    virtual bool RegisterVirtualMachine(const VMDescription &stats) = 0;
    virtual bool AssignTask(int hostId, const ScheduleItem &item) = 0;
    virtual bool CancelTask(int hostId, const ScheduleItem &item) = 0;
    virtual bool ResetSchedule(const Schedule &s) = 0;
};
