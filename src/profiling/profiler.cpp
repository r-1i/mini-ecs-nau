#include "profiler.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <utility>

void Profiler::record(const std::string& name, double ms)
{
    Entry& entry = entries()[name];
    entry.callCount++;
    entry.totalMs += ms;
    entry.minMs = std::min(entry.minMs, ms);
    entry.maxMs = std::max(entry.maxMs, ms);
}

void Profiler::printReport()
{
    std::cout << "\n--- Profiler report ---\n";
    std::cout << std::left << std::setw(28) << "Scope" << std::right
              << std::setw(10) << "Calls" << std::setw(14) << "Total(ms)"
              << std::setw(12) << "Avg(ms)" << std::setw(12) << "Min(ms)"
              << std::setw(12) << "Max(ms)" << "\n";
    for (const auto& [name, entry] : entries())
    {
        std::cout << std::left << std::setw(28) << name << std::right
                  << std::setw(10) << entry.callCount << std::setw(14)
                  << std::fixed << std::setprecision(3) << entry.totalMs
                  << std::setw(12) << (entry.totalMs / entry.callCount)
                  << std::setw(12) << entry.minMs << std::setw(12)
                  << entry.maxMs << "\n";
    }
}

std::unordered_map<std::string, Profiler::Entry>& Profiler::entries()
{
    static std::unordered_map<std::string, Entry> s_entries;
    return s_entries;
}

ScopedTimer::ScopedTimer(std::string name)
    : m_name(std::move(name)), m_start(std::chrono::steady_clock::now())
{
}

ScopedTimer::~ScopedTimer()
{
    const auto end = std::chrono::steady_clock::now();
    const double ms =
        std::chrono::duration<double, std::milli>(end - m_start).count();
    Profiler::record(m_name, ms);
}
