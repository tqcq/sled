#include "sled/futures/internal/failure_handling.h"

namespace sled {
namespace detail {
static thread_local sled::any last_failure;

bool
HasLastFailure() noexcept
{
    return last_failure.has_value();
}

void
InvalidateLastFailure() noexcept
{
    last_failure.reset();
}

const sled::any &
LastFailureAny() noexcept
{
    return last_failure;
}

void
SetLastFailure(const sled::any &failure) noexcept
{
    last_failure = failure;
}

}// namespace detail
}// namespace sled
