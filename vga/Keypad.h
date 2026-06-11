#ifndef VGA_KEYPAD_H
#define VGA_KEYPAD_H

#include "VGAConstants.h"

#include <cstdint>
#include <atomic>

// ============================================================================
// Keypad — 16-key state register (MMIO at 0xFFFF0004).
//
// Maps physical keypad keys K1–K16 to bits 0–15 of a uint16_t register.
// Also tracks the RESET (R) and STOP (S) control keys as separate flags.
// ============================================================================
class Keypad {
public:
    Keypad() noexcept = default;

    void pressKey(uint8_t pos) noexcept
    { m_state.fetch_or(1u << pos, std::memory_order_relaxed); }

    void releaseKey(uint8_t pos) noexcept
    { m_state.fetch_and(~(1u << pos), std::memory_order_relaxed); }

    void pressReset() noexcept
    { m_reset.store(KeyState::Pressed, std::memory_order_relaxed); }

    void releaseReset() noexcept
    { m_reset.store(KeyState::Released, std::memory_order_relaxed); }

    void pressStop()   noexcept
    { m_stop.store(KeyState::Pressed, std::memory_order_relaxed); }

    void releaseStop()  noexcept
    { m_stop.store(KeyState::Released, std::memory_order_relaxed); }

    [[nodiscard]] uint16_t state() const noexcept
    { return m_state.load(std::memory_order_relaxed); }

    [[nodiscard]] bool isResetPressed() const noexcept
    { return m_reset.load(std::memory_order_relaxed) == KeyState::Pressed; }

    [[nodiscard]] bool isStopPressed() const noexcept
    { return m_stop.load(std::memory_order_relaxed) == KeyState::Pressed; }

private:
    std::atomic<uint16_t> m_state{0};
    std::atomic<KeyState> m_reset{KeyState::Released};
    std::atomic<KeyState> m_stop{KeyState::Released};
};

#endif // VGA_KEYPAD_H
