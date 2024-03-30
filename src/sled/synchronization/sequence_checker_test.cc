#include <sled/log/log.h>
#include <sled/make_unique.h>
#include <sled/synchronization/event.h>
#include <sled/synchronization/sequence_checker.h>
#include <sled/system/thread.h>

void
RunOnDifferentThread(std::function<void()> func)
{
    sled::Event thread_has_run_event;
    std::thread thread([&] {
        func();
        thread_has_run_event.Set();
    });
    thread.detach();
    CHECK(thread_has_run_event.Wait(sled::TimeDelta::Seconds(1)));
}

TEST_SUITE("SequenceChecker")
{

    TEST_CASE("CallsAllowedOnSameThread")
    {
        sled::SequenceChecker sequence_checker;
        CHECK(sequence_checker.IsCurrent());
    }

    TEST_CASE("DestructorAllowedOnDifferentThread")
    {
        auto sequence_checker = sled::MakeUnique<sled::SequenceChecker>();
        RunOnDifferentThread([&] { sequence_checker.reset(); });
    }

    TEST_CASE("Detach")
    {
        sled::SequenceChecker sequence_checker;
        sequence_checker.Detach();

        RunOnDifferentThread([&] { CHECK(sequence_checker.IsCurrent()); });
    }

    TEST_CASE("OnlyCurrentOnOneThread")
    {
        sled::SequenceChecker sequence_checker(sled::SequenceChecker::kDetached);
        RunOnDifferentThread([&] {
            CHECK(sequence_checker.IsCurrent());
            RunOnDifferentThread([&] { CHECK_FALSE(sequence_checker.IsCurrent()); });
            CHECK(sequence_checker.IsCurrent());
        });
    }

    TEST_CASE("DeatchFromThreadAndUseOnTaskQueue")
    {
        auto queue = sled::Thread::Create();
        REQUIRE(queue->Start());
        sled::SequenceChecker sequence_checker;
        sequence_checker.Detach();
        queue->BlockingCall([&] { CHECK(sequence_checker.IsCurrent()); });
    }

    TEST_CASE("DetachFromTaskQueueAndUseOnThread")
    {
        auto queue = sled::Thread::Create();
        REQUIRE(queue->Start());
        queue->BlockingCall([&] {
            sled::SequenceChecker sequence_checker;
            sequence_checker.Detach();
            RunOnDifferentThread([&] { CHECK(sequence_checker.IsCurrent()); });
        });
        queue->Stop();
    }
}
