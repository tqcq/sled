#include "sled/system/pid.h"

namespace sled {
ProcessId
GetCachedPID()
{
    static pid_t cached_pid = getpid();
    return cached_pid;
}
}// namespace sled
