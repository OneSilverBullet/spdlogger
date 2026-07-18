A tiny project that uses spdlog.
Build:
  cmake -S . -B build
  cmake --build build -j
  ./build/spdlogger --seconds 5 (optional --stress)


## Development Workflow

Since the codebase was relatively small, I first reviewed each header file to understand the intended behavior of its run() function.

I then followed this workflow:
- Identified and fixed the correctness issues in non-stress mode.
- Fixed the issues that only appeared in stress mode.
- Performed final validation using AddressSanitizer (ASan) and ThreadSanitizer (TSan).

## Verification

The current code was built and exercised in stress mode with both AddressSanitizer and ThreadSanitizer. No memory-safety errors, invalid memory accesses, or data races were detected during these runs.

### AddressSanitizer
```bash
cmake -S . -B asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"
cmake --build asan -j
./asan/spdlogger --seconds 5 --stress
```
### ThreadSanitizer
```bash
cmake -S . -B tsan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=thread -fno-omit-frame-pointer -g" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread"
cmake --build tsan -j
./tsan/spdlogger --seconds 5 --stress
```
## Correctness fixes

### 1. Shared log level race

**Problem:** One thread read `g_level` while another modified it, causing undefined behavior.

**Why intermittent:** The observed value depended on scheduling, CPU caches, and compiler optimizations.

**Fix:** Make the value atomic. Relaxed atomic operations are the best fit because the value is only a counter and does not synchronize other data.

### 2. Debug messages were filtered out

**Problem:** Debug messages were hidden by the global `info` threshold.

**Why:** This was a configuration issue, not a failure to call the logging function.

**Fix:** Enable the `debug` threshold globally. This exposes the messages without incorrectly changing their severity.

### 3. Unsafe format-string input

**Problem:** Dynamic text could be interpreted as formatting syntax instead of plain log data.

**Why intermittent:** The issue only appeared when the text contained formatting characters such as braces.

**Fix:** Use a fixed format string with a placeholder. This cleanly separates the format from untrusted or variable content.

### 4. Invalid string deletion

**Problem:** A `std::string` member was manually deleted even though it was not separately allocated.

**Why intermittent:** The invalid path was conditional, and its effect depended on allocator state.

**Fix:** Use the default destructor. RAII already gives the member exactly one correct and automatic destruction path.

### 5. Deferred string use-after-free

**Problem:** Deferred logging kept a pointer to a temporary string that had already been destroyed.

**Why intermittent:** Stale memory can appear valid until another operation reuses or overwrites it.

**Fix:** Let the deferred object own the string. Ownership directly matches the required lifetime and removes the dangling pointer entirely.

### 6. Invalid elapsed-time calculation

**Problem:** Timestamps from clocks with unrelated time bases were subtracted.

**Why intermittent:** Platform clock implementations and wall-clock adjustments changed the apparent result.

**Fix:** Use `steady_clock` for both timestamps. It is monotonic and specifically designed for measuring elapsed time.

### 7. Non-owning logger pointer

**Problem:** A worker used a raw logger pointer whose lifetime it could not guarantee.

**Why intermittent:** Safety depended on whether another thread released the last owning reference first.

**Fix:** Retain a `shared_ptr` while the worker is active. Shared ownership precisely guarantees that the logger remains alive for every use by that worker.

### 8. Worker stopped unrelated modules

**Problem:** The shutdown worker modified a stop flag shared by all workers, ending unrelated work early.

**Why intermittent:** The amount of work completed before stopping depended on thread startup order.

**Fix:** Give `main` sole ownership of the shared stop decision. A single lifecycle owner makes shutdown timing deterministic and prevents modules from controlling each other.

### 9. Logger shut down while still in use

**Problem:** Stress mode could destroy the logging registry while producer threads were still logging.

**Why intermittent:** A crash required an unlucky overlap between a log call and shutdown.

**Fix:** Stop and join every producer before calling `spdlog::shutdown()`. This ordering is the strongest fix because it removes the unsafe overlap instead of trying to tolerate it:
