#ifndef SLED_EVENT_BUS_EVENT_BUS_H
#define SLED_EVENT_BUS_EVENT_BUS_H

#include "sled/exec/detail/invoke_result.h"
#include "sled/sigslot.h"
#include "sled/synchronization/mutex.h"
#include <typeindex>

namespace sled {

class EventBus;
class Subscriber;

namespace internal {
template<typename Event>
using RawType = typename std::remove_cv<typename std::remove_reference<Event>::type>::type;
template<typename T>
using EnableNotVolatile = typename std::enable_if<!std::is_volatile<T>::value>;
// using RawType = typename std::remove_const<typename std::remove_reference<Event>::type>::type;
void IncrementEvenetRegistryCount();
int GetEventRegistryCount();

template<typename Event>
class EventRegistry {
public:
    static_assert(!std::is_const<Event>::value, "Event type must be non-const");
    static_assert(!std::is_volatile<Event>::value, "Event type must be non-volatile");
    static_assert(!std::is_reference<Event>::value, "Event type must be non-reference");
    using Dispatcher      = sigslot::signal1<Event>;
    using SubscriberTable = std::unordered_map<EventBus *, Dispatcher>;

    EventRegistry() { internal::IncrementEvenetRegistryCount(); }

    static EventRegistry &Instance()
    {
        static EventRegistry registry;
        return registry;
    }

    static std::function<void(EventBus *)> &GetCleanupHandler()
    {
        static std::function<void(EventBus *)> cleanup_handler
            = std::bind(&EventRegistry::OnBusDestroyed, &Instance(), std::placeholders::_1);
        return cleanup_handler;
    }

    void Post(EventBus *bus, const Event &event)
    {
        sled::SharedMutexReadLock lock(&shared_mutex_);
        if (signals_.empty()) { return; }
        auto iter = signals_.find(bus);
        if (iter != signals_.end()) { iter->second(event); }
    }

    template<typename C, typename F>
    void Subscribe(EventBus *bus, C *instance, F &&method)
    {
        sled::SharedMutexWriteLock lock(&shared_mutex_);
        bool is_empty = signals_.empty();
        {
            auto iter = signals_.find(bus);
            if (iter == signals_.end()) {
                signals_.emplace(bus, Dispatcher());
                iter = signals_.find(bus);
            }
            auto &dispatcher = iter->second;
            dispatcher.connect(instance, method);
        }
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

}// namespace internal

class EventBus {
public:
    using MultiThreadedLocal  = sigslot::multi_threaded_local;
    using MultiThreadedGlobal = sigslot::multi_threaded_global;

    template<typename mt_policy = MultiThreadedLocal>
    using Subscriber = sigslot::has_slots<mt_policy>;

    static int EventRegistryCount() { return internal::GetEventRegistryCount(); }

    EventBus() = default;

    ~EventBus()
    {
        for (const auto &handler : cleanup_handlers_) { handler.second(this); }
    }

    EventBus(const EventBus &)            = delete;
    EventBus &operator=(const EventBus &) = delete;

    template<typename Event>
    void Post(Event &&event)
    {
        using U = typename internal::RawType<Event>;
        internal::EventRegistry<U>::Instance().Post(this, std::forward<Event>(event));
    }

    template<typename Event, typename From>
    void PostTo(From &&value)
    {
        using U = typename internal::RawType<Event>;
        internal::EventRegistry<U>::Instance().Post(this, std::forward<From>(value));
    }

    // On<Event1> ([](const Event1 &){})
    template<typename Event, typename C>
    typename std::enable_if<std::is_base_of<sigslot::has_slots_interface, C>::value>::type
    Subscribe(C *instance, void (C::*method)(Event))
    {
        using U = typename internal::RawType<Event>;
        {
            sled::MutexLock lock(&mutex_);
            auto iter = cleanup_handlers_.find(std::type_index(typeid(U)));
            if (iter == cleanup_handlers_.end()) {
                cleanup_handlers_[std::type_index(typeid(U))] = internal::EventRegistry<U>::GetCleanupHandler();
            }
        }

        internal::EventRegistry<U>::Instance().Subscribe(this, instance, method);
    }

    template<typename Event, typename C>
    typename std::enable_if<std::is_base_of<sigslot::has_slots_interface, C>::value>::type Unsubscribe(C *instance)
    {
        using U = typename internal::RawType<Event>;
        internal::EventRegistry<U>::Instance().Unsubscribe(this, instance);
        {
            sled::MutexLock lock(&mutex_);
            if (internal::EventRegistry<U>::Instance().IsEmpty(this)) {
                auto iter = cleanup_handlers_.find(std::type_index(typeid(U)));
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
