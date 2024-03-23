#ifndef SLED_PROFILING_PROFILING_H
#define SLED_PROFILING_PROFILING_H
#pragma once

#include "sled/system/location.h"
#include <fmt/format.h>
#include <string>

namespace sled {
class Profiling {
public:
    static Profiling *Instance();

    inline bool Started() const { return started_; }

    bool Start(const std::string &file);
    void Stop();

    void TimeBegin(const std::string &name);
    void TimeEnd(const std::string &name);

    void StartProcessMemoryMonitoring(int period_ms);
    void StartSystemMemoryMonitoring(int period_ms);
    void StartCPUUsageMonitoring(int period_ms);
    static void GetSystemMemory(int &total_mem, int &available_mem, int &free_mem);
    static void GetProcessMemory(int &rss, int &shared);

private:
    bool started_ = false;
};

class FunctionSampler final {
public:
    inline FunctionSampler(const Location &location = Location::Current())
    {
        std::string filename = location.file();
        key_ =
            fmt::format("{}:{}@{}", filename.substr(filename.find_last_of('/') + 1), location.line(), location.func());
        Profiling::Instance()->TimeBegin(key_);
    }

    inline ~FunctionSampler() { Profiling::Instance()->TimeEnd(key_); }

private:
    std::string key_;
};

}// namespace sled
#endif// SLED_PROFILING_PROFILING_H
