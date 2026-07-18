
#pragma once
#include <spdlog/spdlog.h>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>

namespace string_uaf {

struct DeferredLog {
  // FIX START [julien.zhang]: Own the message so it remains valid until emit() is called.
  //const char* ptr;
  //size_t len;
  std::string message;
  // FIX END [julien.zhang]

  void set() {
    // FIX START [julien.zhang]: Own the message so it remains valid until emit() is called.
    //std::string tmp = "uaf:" + std::to_string(std::rand());
    //ptr = tmp.data();
    //len = tmp.size();
    message = "uaf:" + std::to_string(std::rand());
    // FIX END [julien.zhang]
  }

  void emit() const {
    // FIX START [julien.zhang]: Own the message so it remains valid until emit() is called.
    spdlog::info("deferred={}", message);
    // FIX END [julien.zhang]
  }
};

inline void run(std::atomic<bool>& stop, bool stress) {
  while (!stop.load()) {
    DeferredLog d;
    d.set();
    std::this_thread::sleep_for(std::chrono::microseconds(stress ? 50 : 500));
    d.emit();
    std::this_thread::sleep_for(std::chrono::milliseconds(stress ? 1 : 10));
  }
}
}
