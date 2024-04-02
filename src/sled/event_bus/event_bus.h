#ifndef SLED_EVENT_BUS_EVENT_BUS_H
#define SLED_EVENT_BUS_EVENT_BUS_H

#include "sled/sigslot.h"
#include "sled/synchronization/mutex.h"
#include <typeindex>

namespace sled {

class EventBus;
class Subscriber;

namespace {

template<typename Event>
class EventRegistry {
public:
    using Dispatcher      = sigslot::signal1<Event>;
    using SubscriberTable = std::unordered_map<EventBus *, Dispatcher>;

    static EventRegistry &Instance()
    {
        static EventRegistry instance_;
        return instance_;
    }

    static std::function<void(EventBus *)> &GetCleanupHandler()
    {
        static std::function<void(EventBus *)> cleanup_handler
            = std::bind(&EventRegistry::OnBusDestroyed, &Instance(), std::placeholders::_1);
        return cleanup_handler;
    }

    void Post(EventBus *bus, Event event)
    {
        sled::SharedMutexReadLock lock(&shared_mutex_);
        if (signals_.empty()) { return; }
        auto iter = signals_.find(bus);
        if (iter != signals_.end()) { iter->second(event); }
    }

    template<typename C>
    void Subscribe(EventBus *bus, C *instance, void (C::*method)(Event))
    {
        sled::SharedMutexWriteLock lock(&shared_mutex_);
        auto iter = signals_.find(bus);
        if (iter == signals_.end()) {
            signals_.emplace(bus, Dispatcher());
            iter = signals_.find(bus);
        }
        auto &dispatcher = iter->second;
        dispatcher.connect(instance, method);
    }

    template<typename C>
    void Unsubscribe(EventBus *bus, C *instance)
    {
        sled::SharedMutexWriteLock lock(&shared_mutex_);
        auto iter = signals_.find(bus);
        if (iter == signals_.end()) { return; }
        auto &dispatcher = iter->second;
        dispatcher.disconnect(instance);
        if (dispatcher.is_empty()) { signals_.erase(iter); }
    }

    bool IsEmpty(EventBus *bus) const
    {
        sled::SharedMutexReadLock lock(&shared_mutex_);
        auto iter = signals_.find(bus);
        if (iter == signals_.end()) { return true; }
        return iter->second.is_empty();
    }

    void OnBusDestroyed(EventBus *bus)
    {
        sled::SharedMutexWriteLock lock(&shared_mutex_);
        signals_.erase(bus);
    }

    template<typename C>
    void OnSubscriberDestroyed(C *instance)
    {
        sled::SharedMutexWriteLock lock(&shared_mutex_);
        for (auto &entry : signals_) {
            auto &dispatcher = entry.second;
            dispatcher.disconnect(instance);
        }
    }

private:
    mutable sled::SharedMutex shared_mutex_;

    SubscriberTable signals_;
};

}// namespace

class EventBus {
public:
    class Subscriber : public sigslot::has_slots<> {
    public:
        virtual ~Subscriber() = default;
    };

    EventBus() = default;

    ~EventBus()
    {
        for (const auto &handler : cleanup_handlers_) { handler.second(this); }
    }

    EventBus(const EventBus &)            = delete;
    EventBus &operator=(const EventBus &) = delete;

    template<typename Event>
    void Post(const Event &event)
    {
        EventRegistry<Event>::Instance().Post(this, event);
    }

    // On<Event1> ([](const Event1 &){})
    template<typename Event, typename C>
    typename std::enable_if<std::is_base_of<Subscriber, C>::value>::type
    Subscribe(C *instance, void (C::*method)(Event))
    {
        {
            sled::MutexLock lock(&mutex_);
            cleanup_handlers_[std::type_index(typeid(Event))] = EventRegistry<Event>::GetCleanupHandler();
        }

        EventRegistry<Event>::Instance().Subscribe(this, instance, method);
    }

    template<typename Event, typename C>
    typename std::enable_if<std::is_base_of<Subscriber, C>::value>::type Unsubscribe(C *instance)
    {
        EventRegistry<Event>::Instance().Unsubscribe(this, instance);
        {
            sled::MutexLock lock(&mutex_);
            if (EventRegistry<Event>::Instance().IsEmpty(this)) {
                auto iter = cleanup_handlers_.find(std::type_index(typeid(Event)));
                if (iter != cleanup_handlers_.end()) {
                    iter->second(this);
                    cleanup_handlers_.erase(iter);
                }
            }
        }
    }

private:
    sled::Mutex mutex_;
    std::unordered_map<std::type_index, std::function<void(EventBus *)>> cleanup_handlers_ GUARDED_BY(mutex_);
};
}// namespace sled
#endif// SLED_EVENT_BUS_EVENT_BUS_H
