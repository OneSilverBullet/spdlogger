
#pragma once
#include <spdlog/spdlog.h>
#include <atomic>
#include <thread>

namespace shutdown {
inline void run(std::atomic<bool>& stop, bool stress) {
  spdlog::logger* raw = spdlog::default_logger_raw();
  std::thread t([&] {
    while (!stop.load()) raw->info("shutdown now");
  });

  if (stress) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // FIX START [julien.zhang]: Defer shutdown to main until all logging threads have stopped and joined.
    // spdlog::shutdown();
    // FIX END [julien.zhang]
  }

  // FIX START [julien.zhang]: Let main manage the shared stop flag to avoid stopping all worker threads prematurely.
  // stop.store(true);
  // FIX END [julien.zhang]
  t.join();
}
}
