#ifndef VGA_COLOR_H
#define VGA_COLOR_H

#include "VGAConstants.h"

#include <array>
#include <algorithm>
#include <cstdint>
#include <span>

// ============================================================================
// VGAColor — 24-bit RGB color derived from a 4-bit VGA palette entry.
//
// The VGA text-mode palette uses 4-bit codes (0x0–0xF).  Each code maps to
// an 18-bit DAC value on real hardware.  We expand to 24-bit RGB for SDL:
//   bits 7-6 → red   (3 bits → upper 3 bits of R)
//   bits 5-3 → green (3 bits → upper 3 bits of G)
//   bits 2-0 → blue  (2 bits → upper 2 bits of B)
// ============================================================================
class VGAColor {
public:
    constexpr VGAColor() noexcept = default;
    constexpr explicit VGAColor(uint8_t vgaColor) noexcept { setFromVGA(vgaColor); }

    VGAColor& operator=(uint8_t vgaColor) noexcept {
        setFromVGA(vgaColor);
        return *this;
    }

    [[nodiscard]] constexpr uint8_t red()   const noexcept { return m_red; }
    [[nodiscard]] constexpr uint8_t green() const noexcept { return m_green; }
    [[nodiscard]] constexpr uint8_t blue()  const noexcept { return m_blue; }

private:
    constexpr void setFromVGA(uint8_t color) noexcept {
        m_red   = (color & 0xE0);            // 3 bits → upper 3 bits of red
        m_green = (color & 0x1C) << 3;       // 3 bits → upper 3 bits of green
        m_blue  = (color & 0x03) << 6;       // 2 bits → upper 2 bits of blue
    }

    uint8_t m_red{};
    uint8_t m_green{};
    uint8_t m_blue{};
};

// ============================================================================
// VGAColorPalette — fixed-size array of 16 VGAColor entries.
// ============================================================================
class VGAColorPalette {
public:
    VGAColorPalette() noexcept = default;

    /// Load palette entries from ROM data (one 4-bit VGA code per byte).
    void loadFromRomData(std::span<const uint8_t> romColors) {
        const auto count = std::min(romColors.size(),
                                     static_cast<size_t>(VGA_NUM_COLORS));
        for (size_t i = 0; i < count; ++i) {
            m_colors[i] = VGAColor{romColors[i]};
        }
    }

    [[nodiscard]] const VGAColor& operator[](size_t index) const noexcept {
        return m_colors[index];
    }

    [[nodiscard]] constexpr size_t size() const noexcept { return m_colors.size(); }

private:
    std::array<VGAColor, VGA_NUM_COLORS> m_colors{};
};

#endif // VGA_COLOR_H
