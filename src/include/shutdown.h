
#pragma once
#include <spdlog/spdlog.h>
#include <atomic>
#include <thread>

namespace shutdown {
inline void run(std::atomic<bool>& stop, bool stress) {
  // FIX START [julien.zhang]: Prevent using raw pointer.
  auto default_logger_pointer = spdlog::default_logger();
  // FIX END [julien.zhang] 
  std::thread t([&] {
    while (!stop.load()) default_logger_pointer->info("shutdown now");
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
