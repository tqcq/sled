#include "sled/system/thread_pool.h"

namespace sled {
ThreadPool::ThreadPool(int num_threads)
{
    if (num_threads == -1) {
        num_threads = std::thread::hardware_concurrency();
    }
    scheduler = new sled::Scheduler(
        sled::Scheduler::Config().setWorkerThreadCount(num_threads));
}

ThreadPool::~ThreadPool() { delete scheduler; }

}// namespace sled
