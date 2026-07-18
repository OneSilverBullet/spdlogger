
#pragma once
#include <spdlog/spdlog.h>
#include <atomic>
#include <chrono>
#include <thread>

namespace timebase {
inline void run(std::atomic<bool>& stop, bool) {
  while (!stop.load()) {
    auto t1 = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    // FIX START [julien.zhang]: Use the same monotonic clock for both timestamps to measure elapsed time correctly.
    auto t2 = std::chrono::steady_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    // FIX END [julien.zhang]
    spdlog::info("latency={} us", us);
  }
}
}
