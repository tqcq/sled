#include <sled/event_bus/event_bus.h>
#include <sled/log/log.h>
#include <sled/system/fiber/wait_group.h>
#include <sled/system/thread_pool.h>

using namespace fakeit;

template<typename T, typename R, typename... Args>
void *
GetPtr(R (T::*p)(Args...))
{
    union {
        R (T::*ptr)(Args...);
        void *void_ptr;
    } _;

    _.ptr = p;
    return _.void_ptr;
}

struct Event1 {
    int a;
};

struct Event2 {
    std::string str;
};

struct Subscriber : public sled::EventBus::Subscriber<> {
    void OnEvent1(Event1 event) { a += event.a; }

    void OnEvent2(Event2 event) { str += event.str; }

    int a           = 0;
    std::string str = "";
};

TEST_SUITE("EventBus")
{
    TEST_CASE("RawType built-in type")
    {
        CHECK(std::is_same<int, sled::internal::RawType<int>>::value);
        CHECK(std::is_same<int, sled::internal::RawType<const int>>::value);
        CHECK(std::is_same<int, sled::internal::RawType<const int &>>::value);
        CHECK(std::is_same<int, sled::internal::RawType<int &>>::value);
        CHECK(std::is_same<int, sled::internal::RawType<int &&>>::value);

        CHECK(std::is_same<int, sled::internal::RawType<volatile int>>::value);
        CHECK(std::is_same<int, sled::internal::RawType<volatile const int>>::value);
        CHECK(std::is_same<int, sled::internal::RawType<volatile const int &>>::value);
        CHECK(std::is_same<int, sled::internal::RawType<volatile int &>>::value);
        CHECK(std::is_same<int, sled::internal::RawType<volatile int &&>>::value);
    }

    TEST_CASE("RawType user-defined type")
    {
        struct A {};

        CHECK(std::is_same<A, sled::internal::RawType<A>>::value);
        CHECK(std::is_same<A, sled::internal::RawType<const A>>::value);
        CHECK(std::is_same<A, sled::internal::RawType<const A &>>::value);
        CHECK(std::is_same<A, sled::internal::RawType<A &>>::value);
        CHECK(std::is_same<A, sled::internal::RawType<A &&>>::value);

        CHECK(std::is_same<A, sled::internal::RawType<volatile A>>::value);
        CHECK(std::is_same<A, sled::internal::RawType<volatile const A>>::value);
        CHECK(std::is_same<A, sled::internal::RawType<volatile const A &>>::value);
        CHECK(std::is_same<A, sled::internal::RawType<volatile A &>>::value);
        CHECK(std::is_same<A, sled::internal::RawType<volatile A &&>>::value);
    }

    TEST_CASE("single thread")
    {
        sled::EventBus bus;
        bus.Post(Event1{1});
        bus.Post(Event2{"1"});

        Subscriber subscriber;
        bus.Subscribe<Event1>(&subscriber, &Subscriber::OnEvent1);
        bus.Subscribe<Event2>(&subscriber, &Subscriber::OnEvent2);

        bus.Post(Event1{1});
        bus.Post(Event2{"1"});

        CHECK_EQ(subscriber.a, 1);
        CHECK_EQ(subscriber.str, "1");

        bus.Post(Event1{1});
        bus.Post(Event2{"1"});

        CHECK_EQ(subscriber.a, 2);
        CHECK_EQ(subscriber.str, "11");
    }

    TEST_CASE("multi thread")
    {
        auto thread = sled::Thread::Create();
        thread->Start();

        sled::EventBus bus;
        Subscriber subscriber;

        bus.Subscribe(&subscriber, &Subscriber::OnEvent1);
        bus.Subscribe(&subscriber, &Subscriber::OnEvent2);

        thread->BlockingCall([&] {
            bus.Post(Event1{1});
            bus.Post(Event2{"1"});
        });

        CHECK_EQ(subscriber.a, 1);
        CHECK_EQ(subscriber.str, "1");
    }

    TEST_CASE("thread_pool")
    {
        constexpr int kPublishCount    = 10;
        constexpr int kSubscriberCount = 1000;

        struct AtomicEvent {
            std::atomic<int> &data;
        };

        struct AotmicEventSubscriber : public sled::EventBus::Subscriber<> {
            virtual ~AotmicEventSubscriber() = default;

            void OnEvent(AtomicEvent event)
            {
                ++a;
                event.data.fetch_add(1);
            }

            int a = 0;
        };

        std::atomic<int> value(0);
        AtomicEvent atomic_event{value};

        sled::WaitGroup wg(kPublishCount);
        sled::ThreadPool pool(kPublishCount);
        sled::EventBus bus;
        std::vector<AotmicEventSubscriber> subscribers(kSubscriberCount);

        for (auto &sub : subscribers) { bus.Subscribe(&sub, &AotmicEventSubscriber::OnEvent); }
        std::atomic<int> invoke_count(0);
        for (int i = 0; i < kPublishCount; i++) {
            pool.PostTask([wg, atomic_event, &bus, &invoke_count]() {
                bus.Post(atomic_event);
                invoke_count.fetch_add(1);
                wg.Done();
            });
        }
        wg.Wait();

        CHECK_EQ(invoke_count.load(), kPublishCount);
        CHECK_EQ(value.load(), kPublishCount * kSubscriberCount);
    }

    TEST_CASE("same type")
    {
        struct Event {
            Event(int v) : a(v) {}

            int a;
        };

        struct Subscriber : public sled::EventBus::Subscriber<> {
            void OnEvent(Event event) { a += event.a; }

            int a = 0;
        };

        sled::EventBus bus;
        Subscriber subscriber1;
        Subscriber subscriber2;
        bus.Subscribe<Event>(&subscriber1, &Subscriber::OnEvent);
        bus.Subscribe<Event>(&subscriber2, &Subscriber::OnEvent);
        bus.Post(Event{1});
        CHECK_EQ(subscriber1.a, 1);
        CHECK_EQ(subscriber2.a, 1);
        Event e{1};
        bus.Post(e);
        CHECK_EQ(subscriber1.a, 2);
        CHECK_EQ(subscriber2.a, 2);
        bus.PostTo<Event>(1);
        CHECK_EQ(subscriber1.a, 3);
        CHECK_EQ(subscriber2.a, 3);
    }
}
