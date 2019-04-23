#include "scheduler_factory.h"
#include "schedulers/genetic_scheduler.h"
#include "schedulers/mao_scheduler.h"

#include <simgrid/s4u.hpp>

XBT_LOG_NEW_DEFAULT_CATEGORY(scheduler_factory, "Scheduler factory log");

using std::string;

SchedulerFactory::SchedulerFactory() {
    RegisterScheduler("mao", &MaoScheduler::Create);
    RegisterScheduler("genetic", &GeneticScheduler::Create);
}

SchedulerFactory& SchedulerFactory::GetInstance() {
    static SchedulerFactory instance;
    return instance;
}

AbstractScheduler* SchedulerFactory::GetScheduler(const string& schedulerName) {
    if (FactoryMap.count(schedulerName)) {
        return FactoryMap[schedulerName]();
    } else {
        XBT_WARN("%s is unrecognized scheduler option!", schedulerName.c_str());
        XBT_WARN("Using default %s scheduler", FactoryMap.begin()->first.c_str());
        return FactoryMap.begin()->second();
    }
}

void SchedulerFactory::RegisterScheduler(string schedulerName, const std::function<AbstractScheduler*()>& schedulerConstructor) {
    FactoryMap[schedulerName] = schedulerConstructor;
}
