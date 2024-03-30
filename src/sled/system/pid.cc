#include "sled/system/pid.h"

namespace sled {
pid_t
GetCachedPID()
{
    static pid_t cached_pid = getpid();
    return cached_pid;
}
}// namespace sled
