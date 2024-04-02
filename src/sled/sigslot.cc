#include "sled/sigslot.h"

namespace sigslot {

#ifdef _SIGSLOT_HAS_POSIX_THREADS

sled::RecursiveMutex *
multi_threaded_global::get_mutex()
{
    static sled::RecursiveMutex g_mutex;
    return &g_mutex;
}

// sled::Mutex *
// multi_threaded_global::get_mutex()
// {
//     static sled::Mutex g_mutex;
//     return &g_mutex;
// }

// pthread_mutex_t *
// multi_threaded_global::get_mutex()
// {
//     static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
//     return &g_mutex;
// }

#endif// _SIGSLOT_HAS_POSIX_THREADS
}// namespace sigslot
