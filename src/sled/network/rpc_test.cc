#include <sled/lang/attributes.h>
#include <sled/network/rpc.h>
#include <sled/random.h>
#include <sled/time_utils.h>

TEST_SUITE("RPC")
{
    TEST_CASE("loopback")
    {
        auto conn       = sled::loopback_connection::create();
        auto rpc_client = sled::rpc::create(conn.first);
        auto rpc_server = sled::rpc::create(conn.second);
        rpc_server->set_timer([](uint32_t ms, const sled::rpc::timeout_cb &cb) {
            SLED_UNUSED(ms);
            SLED_UNUSED(cb);
        });
        rpc_client->set_timer([](uint32_t ms, const sled::rpc::timeout_cb &cb) {
            SLED_UNUSED(ms);
            SLED_UNUSED(cb);
        });
        rpc_server->set_ready(true);
        rpc_client->set_ready(true);
        sled::Random rand(sled::TimeUTCMicros());

        rpc_server->subscribe("cmd", [](int x) { return x + 1; });

        int random_value = rand.Rand(0, 10000);
        rpc_client->cmd("cmd")
            ->msg(random_value)
            ->rsp([random_value](int x) { CHECK_EQ(x, random_value + 1); })
            ->call();
    }
}
