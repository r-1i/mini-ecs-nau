#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>

// Lightweight scope-based time profiler. Not thread-safe by design -
// this project is single-threaded, and adding synchronization here
// would only cost performance without a real use case.
class Profiler {
 public:
  struct Entry {
    uint64_t callCount = 0;
    double totalMs = 0.0;
    double minMs = std::numeric_limits<double>::max();
    double maxMs = 0.0;
  };

  static void Record(const std::string& name, double ms) {
    Entry& entry = Entries()[name];
    entry.callCount++;
    entry.totalMs += ms;
    entry.minMs = std::min(entry.minMs, ms);
    entry.maxMs = std::max(entry.maxMs, ms);
  }

  static void PrintReport() {
    std::cout << "\n--- Profiler report ---\n";
    std::cout << std::left << std::setw(28) << "Scope" << std::right
              << std::setw(10) << "Calls" << std::setw(14) << "Total(ms)"
              << std::setw(12) << "Avg(ms)" << std::setw(12) << "Min(ms)"
              << std::setw(12) << "Max(ms)" << "\n";
    for (const auto& [name, entry] : Entries()) {
      std::cout << std::left << std::setw(28) << name << std::right
                << std::setw(10) << entry.callCount << std::setw(14)
                << std::fixed << std::setprecision(3) << entry.totalMs
                << std::setw(12) << (entry.totalMs / entry.callCount)
                << std::setw(12) << entry.minMs << std::setw(12)
                << entry.maxMs << "\n";
    }
  }

 private:
  // Construct-on-first-use: guarantees the map exists before any
  // ScopedTimer can record into it, regardless of static
  // initialization order across translation units - the same
  // reasoning we discussed for the component registry singleton.
  static std::unordered_map<std::string, Entry>& Entries() {
    static std::unordered_map<std::string, Entry> entries;
    return entries;
  }
};

// RAII scope timer: starts on construction, records elapsed time on
// destruction. Because it's tied to object lifetime, PROFILE_SCOPE
// covers the whole enclosing block correctly even with early
// returns or exceptions - there is no separate stop() to forget.
class ScopedTimer {
 public:
  explicit ScopedTimer(std::string name)
      : name_(std::move(name)), start_(std::chrono::steady_clock::now()) {}

  ~ScopedTimer() {
    const auto end = std::chrono::steady_clock::now();
    const double ms =
        std::chrono::duration<double, std::milli>(end - start_).count();
    Profiler::Record(name_, ms);
  }

  ScopedTimer(const ScopedTimer&) = delete;
  ScopedTimer& operator=(const ScopedTimer&) = delete;

 private:
  std::string name_;
  std::chrono::steady_clock::time_point start_;
};

#define PROFILE_CONCAT_INNER(a, b) a##b
#define PROFILE_CONCAT(a, b) PROFILE_CONCAT_INNER(a, b)
// Declares a uniquely-named ScopedTimer for the current block, using
// __LINE__ to avoid name collisions if used more than once per scope.
#define PROFILE_SCOPE(name) \
  ScopedTimer PROFILE_CONCAT(scopedTimer_, __LINE__)(name)
