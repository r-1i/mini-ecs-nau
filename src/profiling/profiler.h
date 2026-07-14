#pragma once

#include <chrono>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>

/**
 * @brief Lightweight scope-based time profiler.
 *
 * Not thread-safe by design - this project is single-threaded, and
 * adding synchronization here would only cost performance without a
 * real use case.
 */
class Profiler
{
public:
    struct Entry
    {
        uint64_t callCount = 0;
        double totalMs = 0.0;
        double minMs = std::numeric_limits<double>::max();
        double maxMs = 0.0;
    };

    /**
     * @brief Records one timed call under name.
     * @param name Scope name, matched across calls to accumulate stats.
     * @param ms Duration of this call, in milliseconds.
     */
    static void record(const std::string& name, double ms);

    /** @brief Prints call count/total/avg/min/max for every recorded scope. */
    static void printReport();

private:
    // Construct-on-first-use: guarantees the map exists before any
    // ScopedTimer can record into it, regardless of static
    // initialization order across translation units.
    static std::unordered_map<std::string, Entry>& entries();
};

/**
 * @brief RAII scope timer: records elapsed time into Profiler on
 * destruction.
 *
 * Tied to object lifetime, so PROFILE_SCOPE covers the whole enclosing
 * block correctly even with early returns or exceptions - there's no
 * separate stop() to forget.
 */
class ScopedTimer
{
public:
    explicit ScopedTimer(std::string name);
    ~ScopedTimer();

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

private:
    std::string m_name;
    std::chrono::steady_clock::time_point m_start;
};

#define PROFILE_CONCAT_INNER(a, b) a##b
#define PROFILE_CONCAT(a, b) PROFILE_CONCAT_INNER(a, b)
// Declares a uniquely-named ScopedTimer for the current block, using
// __LINE__ to avoid name collisions if used more than once per scope.
#define PROFILE_SCOPE(name) \
    ScopedTimer PROFILE_CONCAT(scopedTimer_, __LINE__)(name)
