#include <sled/nonstd/fsm.h>

enum class State {
    kIdle,
    kStarted,
    kEnd,
};

struct FSMTest : public sled::Fsm<FSMTest, State> {
public:
    FSMTest() : Fsm(State::kIdle) {}

    struct StartEvent {
        StartEvent(bool do_nothing = false) : do_nothing(do_nothing) {}

        bool do_nothing = false;
    };

    struct StopEvent {};

    struct ResetEvent {};

    template<typename E>
    state_type NoTransition(const E &)
    {
        ++error_count;
        return CurrentState();
    }

    int error_count = 0;

private:
    bool StartCheck(const StartEvent &e) const { return !e.do_nothing; }

    void OnStart(const StartEvent &) {}

    void OnStop(const StopEvent &) {}

    void OnReset(const ResetEvent &) {}

public:
    using TransitionTable
        = table<row<State::kIdle, StartEvent, State::kStarted, &FSMTest::OnStart, &FSMTest::StartCheck>,
                row<State::kStarted, StopEvent, State::kEnd, &FSMTest::OnStop>,
                row<State::kEnd, ResetEvent, State::kIdle, &FSMTest::OnReset>>;
};

TEST_SUITE("fsm")
{
    TEST_CASE("Entry Exit")
    {
        FSMTest fsm_test;
        CHECK_EQ(fsm_test.CurrentState(), State::kIdle);
        CHECK_EQ(fsm_test.error_count, 0);

        fsm_test.ProcessEvent(FSMTest::StartEvent(true));
        CHECK_EQ(fsm_test.CurrentState(), State::kIdle);
        CHECK_EQ(fsm_test.error_count, 1);

        fsm_test.ProcessEvent(FSMTest::StartEvent());
        CHECK_EQ(fsm_test.CurrentState(), State::kStarted);
        CHECK_EQ(fsm_test.error_count, 1);

        fsm_test.ProcessEvent(FSMTest::StopEvent());
        CHECK_EQ(fsm_test.CurrentState(), State::kEnd);
        CHECK_EQ(fsm_test.error_count, 1);

        fsm_test.ProcessEvent(FSMTest::ResetEvent());
        CHECK_EQ(fsm_test.CurrentState(), State::kIdle);
        CHECK_EQ(fsm_test.error_count, 1);

        fsm_test.ProcessEvent(FSMTest::ResetEvent());
        CHECK_EQ(fsm_test.CurrentState(), State::kIdle);
        CHECK_EQ(fsm_test.error_count, 2);
    }
}
