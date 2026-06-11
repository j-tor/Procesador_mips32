#pragma once

#include <cstddef>
#include <cstdint>

// VGA Text Mode Constants
inline constexpr unsigned int SYMB_WIDTH      = 8;
inline constexpr unsigned int SYMB_HEIGHT     = 16;
inline constexpr unsigned int BYTES_PER_ROW   = (SYMB_WIDTH + 7) / 8;       // 1
inline constexpr unsigned int SYMB_SIZE_BYTES = BYTES_PER_ROW * SYMB_HEIGHT; // 16
inline constexpr unsigned int ROW_MSB         = 7;
inline constexpr unsigned int VGA_COLS        = 80;
inline constexpr unsigned int VGA_ROWS        = 30;
inline constexpr unsigned int VGA_NUM_COLORS  = 16;
inline constexpr unsigned int VGA_NUM_SYMBOLS = 256;

/// Pixel dimensions of the full display.
inline constexpr int VGA_WINDOW_WIDTH  = VGA_COLS * SYMB_WIDTH;   // 640
inline constexpr int VGA_WINDOW_HEIGHT = VGA_ROWS * SYMB_HEIGHT;  // 480

inline constexpr uint16_t VGA_CELL_CHAR_MASK = 0x00FF;
inline constexpr uint16_t VGA_CELL_FG_MASK   = 0x0F00;
inline constexpr uint16_t VGA_CELL_BG_MASK   = 0xF000;
inline constexpr unsigned VGA_CELL_FG_SHIFT  = 8;
inline constexpr unsigned VGA_CELL_BG_SHIFT  = 12;

enum class KeyState : uint8_t {
    Released,
    Pressed
};
