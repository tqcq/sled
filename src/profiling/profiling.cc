#include "sled/profiling/profiling.h"
#include <uprofile.h>

namespace sled {
Profiling *
Profiling::Instance()
{
    static Profiling instance;
    return &instance;
}

bool
Profiling::Start(const std::string &file)
{
    uprofile::start(file.c_str());
    started_ = true;
    return true;
}

void
Profiling::Stop()
{
    uprofile::stop();
}

void
Profiling::TimeBegin(const std::string &name)
{
    if (!started_) return;
    uprofile::timeBegin(name);
}

void
Profiling::TimeEnd(const std::string &name)
{
    if (!started_) return;
    uprofile::timeEnd(name);
}

void
Profiling::StartProcessMemoryMonitoring(int period_ms)
{
    if (!started_) return;
    uprofile::startProcessMemoryMonitoring(period_ms);
}

void
Profiling::StartSystemMemoryMonitoring(int period_ms)
{
    if (!started_) return;
    uprofile::startSystemMemoryMonitoring(period_ms);
}

void
Profiling::StartCPUUsageMonitoring(int period_ms)
{
    if (!started_) return;
    uprofile::startCPUUsageMonitoring(period_ms);
}

void
Profiling::GetSystemMemory(int &total_mem, int &available_mem, int &free_mem)
{
    uprofile::getSystemMemory(total_mem, available_mem, free_mem);
}

void
Profiling::GetProcessMemory(int &rss, int &shared)
{
    uprofile::getProcessMemory(rss, shared);
}
}// namespace sled
