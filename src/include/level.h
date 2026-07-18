
#pragma once
#include <spdlog/spdlog.h>
#include <atomic>
#include <thread>
#include <string>

namespace level {
// FIX START [julien.zhang]: Make g_level atomic to prevent data races between reader and writer threads.
inline std::atomic<int> g_level{0};
// FIX END [julien.zhang]

class Level5
{
public:
  Level5(std::string ss): s(std::move(ss)) {};
  // FIX START [julien.zhang]: Use the default destructor because s is an owned member and must not be deleted manually.
  ~Level5() = default;
  // FIX END [julien.zhang]

  void print() {
      // FIX START [julien.zhang]: Fix risk by passing input via format string placeholder.
      spdlog::debug("{}", s);
      // FIX END [julien.zhang]
  }

private:
  std::string s;
};

inline void run(std::atomic<bool>& stop, bool) {
  std::thread w([&] {
    // FIX START [julien.zhang]: Make g_level atomic to prevent data races between reader and writer threads.
    while (!stop.load()) g_level.fetch_add(1, std::memory_order_relaxed);
    // FIX END [julien.zhang]
  });
  std::thread r([&] {
    while (!stop.load()) {
      // FIX START [julien.zhang]: Make g_level atomic to prevent data races between reader and writer threads.
      if (g_level.load(std::memory_order_relaxed) & 1) spdlog::info("odd");
      // FIX END [julien.zhang]
      else spdlog::debug("even");
    }
  });
  w.join();
  r.join();
}
}
