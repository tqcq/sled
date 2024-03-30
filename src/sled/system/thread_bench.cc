#include <sled/system/thread.h>

void
ThreadBlockingCallByDefaultSocketServer(picobench::state &s)
{
    auto thread = sled::Thread::CreateWithSocketServer();
    thread->Start();
    for (auto _ : s) {
        (void) thread->BlockingCall([] { return 1; });
    }
}

void
ThreadBlockingCallByNullSocketServer(picobench::state &s)
{
    auto thread = sled::Thread::Create();
    thread->Start();
    for (auto _ : s) {
        (void) thread->BlockingCall([] { return 1; });
    }
}

PICOBENCH(ThreadBlockingCallByDefaultSocketServer);
PICOBENCH(ThreadBlockingCallByNullSocketServer);
