#include <events/event_factory.h>

EventFactory::EventFactory() {

}

static EventFactory& EventFactory::GetInstance() {
    static EventFactory factory;
    return factory;
}

AbstractEvent* EventFactory::GetEvent(EventType t, AbstractEvent::Context* context) {
    return EventMap[t](*context);
}

void EventFactory::RegisterEvent(EventType t, const std::function<AbstractEvent*(AbstractEvent::Context*)>& eventConstructor) {
    EventMap[t] = eventConstructor;
}
