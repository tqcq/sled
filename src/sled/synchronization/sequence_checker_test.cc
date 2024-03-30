#include <gtest/gtest.h>
#include <sled/log/log.h>
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
    EXPECT_TRUE(thread_has_run_event.Wait(sled::TimeDelta::Seconds(1)));
}

TEST(SequenceChecker, CallsAllowedOnSameThread)
{
    sled::SequenceChecker sequence_checker;
    EXPECT_TRUE(sequence_checker.IsCurrent());
}

TEST(SequenceChecker, DestructorAllowedOnDifferentThread)
{
    auto sequence_checker = std::make_unique<sled::SequenceChecker>();
    RunOnDifferentThread([&] { sequence_checker.reset(); });
}

TEST(SequenceChecker, Detach)
{
    sled::SequenceChecker sequence_checker;
    sequence_checker.Detach();

    RunOnDifferentThread([&] { EXPECT_TRUE(sequence_checker.IsCurrent()); });
}

TEST(SequenceChecker, OnlyCurrentOnOneThread)
{
    sled::SequenceChecker sequence_checker(sled::SequenceChecker::kDetached);
    RunOnDifferentThread([&] {
        EXPECT_TRUE(sequence_checker.IsCurrent());
        RunOnDifferentThread([&] { EXPECT_FALSE(sequence_checker.IsCurrent()); });
        EXPECT_TRUE(sequence_checker.IsCurrent());
    });
}

TEST(SequenceChecker, DeatchFromThreadAndUseOnTaskQueue)
{
    auto queue = sled::Thread::Create();
    ASSERT_TRUE(queue->Start());
    sled::SequenceChecker sequence_checker;
    sequence_checker.Detach();
    queue->BlockingCall([&] { EXPECT_TRUE(sequence_checker.IsCurrent()); });
}

TEST(SequenceChecker, DetachFromTaskQueueAndUseOnThread)
{
    auto queue = sled::Thread::Create();
    ASSERT_TRUE(queue->Start());
    queue->BlockingCall([&] {
        sled::SequenceChecker sequence_checker;
        sequence_checker.Detach();
        RunOnDifferentThread([&] { EXPECT_TRUE(sequence_checker.IsCurrent()); });
    });
    queue->Stop();
}
