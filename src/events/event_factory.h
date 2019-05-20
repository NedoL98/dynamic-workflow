#pragma once
#include <map>
#include <events/event.h>
#include <fstream>

class EventFactory {
private:
    EventFactory();
    std::map<EventType, std::function<AbstractEvent*(AbstractEvent::Context*)>> FactoryMap;
    std::ofstream EventLog;

public:
    SchedulerFactory(const SchedulerFactory&) = delete;
    SchedulerFactory& operator = (const SchedulerFactory&) = delete;
    static EventFactory& GetInstance();
    AbstractEvent* GetEvent(EventType t, AbstractEvent::Context* c) const;
    void RegisterEvent(EventType t, const std::function<AbstractEvent*(AbstractEvent::Context*)>& eventConstructor);
};
