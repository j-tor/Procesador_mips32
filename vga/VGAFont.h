#pragma once

#include "VGAConstants.h"
#include "VGAColor.h"

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include <SDL.h>

// ============================================================================
// VGAFontSymbol — a single 8×16 bitmap glyph.
//
// Pre-computes SDL_Point pixel offsets at load time so draw() does zero
// bit-scanning work.  The relative offsets are transformed to absolute
// screen coordinates on each draw call.
// ============================================================================
class VGAFontSymbol {
public:
    VGAFontSymbol() noexcept = default;

    explicit VGAFontSymbol(std::span<const uint8_t, SYMB_SIZE_BYTES> bitmap) {
        loadFromBitmap(bitmap);
    }

    void loadFromBitmap(std::span<const uint8_t, SYMB_SIZE_BYTES> bitmap);
    void draw(SDL_Renderer* renderer, int x, int y,
              const VGAColor& fg, const VGAColor& bg) const;

    [[nodiscard]] size_t pixelCount() const noexcept { return m_pixelOffsets.size(); }

private:
    /// Pre-computed pixel offsets relative to glyph origin.
    /// Stored as SDL_Point directly — no conversion needed at draw time.
    std::vector<SDL_Point> m_pixelOffsets;
    mutable std::vector<SDL_Point> m_screenPoints; // mutable to allow reuse without reallocation
};

// ============================================================================
// VGAFont — fixed-size font with 256 glyphs (8×16 each).
//
// Supports loading from:
//   1. Raw bitmap data  (std::span<const uint8_t>)
//   2. A file            (auto-detects MIF / PSF v1 / PSF v2 / raw binary)
// ============================================================================
class VGAFont {
public:
    VGAFont() noexcept = default;

    /// Load from in-memory raw bitmap data (256 × 16 bytes).
    void loadFromRomData(std::span<const uint8_t> romData);

    /// Load from a MIF file.
    /// @throws std::runtime_error if the file cannot be opened or the Mif file is invalid.
    void loadFromMifFile(const char* path);

    /// Load from a PSF file (v1 or v2).
    /// @throws std::runtime_error if the file cannot be opened or the PSF file is invalid.
    void loadFromPsfFile(const char* path);

    [[nodiscard]] const VGAFontSymbol& operator[](size_t index) const noexcept {
        return m_symbols[index];
    }

    [[nodiscard]] constexpr size_t size() const noexcept { return m_symbols.size(); }

private:
    std::array<VGAFontSymbol, VGA_NUM_SYMBOLS> m_symbols{};
};
