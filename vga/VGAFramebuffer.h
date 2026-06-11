#pragma once

#include <cstdint>
#include <array>
#include <atomic>
#include "VGAConstants.h"

// ============================================================================
// VGAFramebuffer — 80×30 text-mode framebuffer.
//
// Stores 2400 text cells packed as 1200 uint32_t words (2 cells/word).
// Each cell is a 16-bit entry: [bg:4][fg:4][char:8] — matches the MMIO
// layout at address 0xB800 in the SoC memory map (§5.4).
// ============================================================================
class VGAFramebuffer {
public:
    using CellEntry = uint16_t; // [bg:4][fg:4][char:8]
    using CellBuffer = std::array<CellEntry, VGA_ROWS * VGA_COLS>;

    [[nodiscard]] static constexpr CellEntry makeEntry(uint8_t ch, uint8_t fg,
                                                      uint8_t bg) noexcept {
        return static_cast<CellEntry>(
            (static_cast<CellEntry>(bg & 0x0F) << VGA_CELL_BG_SHIFT) |
            (static_cast<CellEntry>(fg & 0x0F) << VGA_CELL_FG_SHIFT) |
            static_cast<CellEntry>(ch)
        );
    }

    VGAFramebuffer() noexcept
    { clear(0x07, 0x00); }

        // Called by SoC after finishing a full frame
    void commitFrame()
    { m_frameReady.store(true, std::memory_order_release); }

    // Called by render thread — atomically swaps THEN reads
    const CellBuffer& swapAndGetFront();

    void clear(uint8_t fg, uint8_t bg) noexcept;

    /// Write a single character at (row, col) — 0-indexed.
    void writePixel(size_t row, size_t col, uint8_t ch, uint8_t fg, uint8_t bg) noexcept;

    /// Write a C-string starting at (row, col).
    void writeString(size_t row, size_t col, const char* str,
                     uint8_t fg, uint8_t bg) noexcept {
        for (size_t i = 0; str[i] != '\0'; ++i) {
            writePixel(row, col + i, static_cast<uint8_t>(str[i]), fg, bg);
        }
    }

private:
    CellBuffer m_buffers[2];
    std::atomic<int> m_frontIndex{0};  // which buffer is "front"
    std::atomic<bool> m_frameReady{false};
};
