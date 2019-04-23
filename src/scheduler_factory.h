#pragma once

#include "prototypes/scheduler.h"

#include <functional>
#include <map>

class SchedulerFactory {
private:
    SchedulerFactory();
    std::map<std::string, std::function<AbstractScheduler*()>> FactoryMap;
public:
    SchedulerFactory(const SchedulerFactory&) = delete;
    void operator = (const SchedulerFactory&) = delete;

    static SchedulerFactory& GetInstance();

    AbstractScheduler* GetScheduler(const std::string& schedulerName);

    void RegisterScheduler(std::string schedulerName, const std::function<AbstractScheduler*()>& schedulerConstructor);
};