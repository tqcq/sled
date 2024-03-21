#pragma once

#ifndef SLED_RX_H
#define SLED_RX_H
// #include "rx-scheduler.hpp"
// #include "rx-lite.hpp"
#include "rx-includes.hpp"
#include "sled/system/fiber/scheduler.h"
#include "sled/time_utils.h"

namespace sled {
using namespace rxcpp;

// struct fiber_scheduler : public schedulers::scheduler_interface {
//     class new_worker : public schedulers::worker_interface {
//     public:
//         new_worker(composite_subscription cs) {}
//
//         ~new_worker() override {}
//
//         clock_type::time_point now() const override { return clock_type::now(); };
//
//         void schedule(const sled::schedulers::schedulable &scbl) const override { return schedule(now(), scbl); };
//
//         void schedule(clock_type::time_point when, const sled::schedulers::schedulable &scbl) const override{};
//     };
//
// public:
//     ~fiber_scheduler() override {}
//
//     clock_type::time_point now() const override
//     {
//         // auto ns = sled::TimeNanos();
//         // return clock_type::time_point(std::chrono::nanoseconds(ns));
//         return clock_type::now();
//     }
//
//     sled::schedulers::worker create_worker(composite_subscription cs) const override
//     {
//         return sled::schedulers::worker(cs, std::make_shared<new_worker>());
//     };
// };
}// namespace sled

#endif//  SLED_RX_H
