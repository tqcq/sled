#include "sled/event_bus/event_bus.h"

namespace sled {
namespace internal {
static std::atomic<int> g_event_registry_count{0};

void
IncrementEvenetRegistryCount()
{
    g_event_registry_count.fetch_add(1, std::memory_order_relaxed);
}

int
GetEventRegistryCount()
{
    return g_event_registry_count.load(std::memory_order_acquire);
}

}// namespace internal
}// namespace sled
