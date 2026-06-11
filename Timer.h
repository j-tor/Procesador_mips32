#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <cstdint>

/// @brief Millisecond counter (MMIO at 0xFFFF0008).
///
/// Computes elapsed time on every read using std::chrono::steady_clock.
/// No background thread, no atomics needed — the clock is monotonic and
/// reading it is a cheap syscall (~50 ns on Linux via vDSO).
///
/// The 32-bit counter wraps at ~49.7 days, matching real hardware behavior.
class Timer {
public:
    Timer() noexcept : m_bootTime(std::chrono::steady_clock::now()) {}

    /// Read the 32-bit millisecond counter (MMIO address 0xFFFF0008).
    [[nodiscard]] uint32_t read() const noexcept {
        using namespace std::chrono;
        const auto elapsed = duration_cast<milliseconds>(
            steady_clock::now() - m_bootTime);
        return static_cast<uint32_t>(elapsed.count() & 0xFFFF'FFFF);
    }

    /// Reset the counter to zero (e.g., on simulation restart).
    void reset() noexcept {
        m_bootTime = std::chrono::steady_clock::now();
    }

private:
    std::chrono::steady_clock::time_point m_bootTime;
};

#endif // TIMER_H
