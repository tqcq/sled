#include <sled/event_bus/event_bus.h>
#include <sled/log/log.h>
#include <sled/system/fiber/wait_group.h>
#include <sled/system/thread_pool.h>

struct Event {
    std::shared_ptr<int> data = std::make_shared<int>(0);
};

struct AtomicEvent {
    AtomicEvent(std::atomic<int> &v) : data(v) {}

    std::atomic<int> &data;
};

struct Subscriber : public sled::EventBus::Subscriber<> {
    void OnEvent(Event event) { (*event.data)++; }

    void OnAtomicnEvent(AtomicEvent event) { event.data.fetch_add(1); }
};

void
BMEventBusPost_1_to_1(picobench::state &s)
{
    sled::EventBus event_bus;
    Subscriber subscriber;
    event_bus.Subscribe(&subscriber, &Subscriber::OnEvent);
    for (auto _ : s) { event_bus.Post(Event{}); }
}

void
BMEventBusPost_1_to_1k(picobench::state &s)
{
    sled::EventBus event_bus;
    std::vector<Subscriber> subscribers(1000);
    for (auto &subscriber : subscribers) { event_bus.Subscribe(&subscriber, &Subscriber::OnEvent); }

    for (auto _ : s) {
        Event event;
        event_bus.Post(event);
        SLED_ASSERT(*event.data == 1000, "");
    }
}

void
BMEventBusPost_10_to_1k(picobench::state &s)
{
    constexpr int kPublishCount    = 10;
    constexpr int kSubscriberCount = 1000;

    sled::EventBus event_bus;
    std::vector<Subscriber> subscribers(kSubscriberCount);
    for (auto &subscriber : subscribers) { event_bus.Subscribe(&subscriber, &Subscriber::OnAtomicnEvent); }
    sled::ThreadPool pool(kPublishCount);

    for (auto _ : s) {
        std::atomic<int> value(0);
        AtomicEvent atomic_event(value);
        sled::WaitGroup wg(kPublishCount);
        for (int i = 0; i < kPublishCount; i++) {
            pool.PostTask([atomic_event, wg, &event_bus]() {
                event_bus.Post(atomic_event);
                wg.Done();
            });
        }
        wg.Wait();
        SLED_ASSERT(value.load() == kPublishCount * kSubscriberCount,
                    "{} != {}",
                    value.load(),
                    kPublishCount * kSubscriberCount);
    }
}

PICOBENCH_SUITE("EventBus");

PICOBENCH(BMEventBusPost_1_to_1);
PICOBENCH(BMEventBusPost_1_to_1k);
PICOBENCH(BMEventBusPost_10_to_1k);
