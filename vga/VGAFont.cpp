#include <cctype>
#include <algorithm>
#include <format>
#include <string_view>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "VGAFont.h"

void VGAFontSymbol::loadFromBitmap(std::span<const uint8_t, SYMB_SIZE_BYTES> bitmap) {
    m_pixelOffsets.clear();
    m_pixelOffsets.reserve(SYMB_WIDTH * SYMB_HEIGHT / 4);
    
    for (size_t row = 0; row < SYMB_HEIGHT; ++row) {
        const uint8_t lineData = bitmap[row * BYTES_PER_ROW];
        uint8_t mask = 1u << ROW_MSB;

        for (size_t col = 0; col < SYMB_WIDTH; ++col) {
            if ((lineData & mask) != 0) {
                m_pixelOffsets.push_back({ static_cast<int>(col), static_cast<int>(row) });
            }
            mask >>= 1;
        }
    }
    m_screenPoints.resize(m_pixelOffsets.size()); // trim excess capacity
}

void VGAFontSymbol::draw(SDL_Renderer* renderer, int x, int y,
                         const VGAColor& fg, const VGAColor& bg) const {
    const SDL_Rect bgRect{x, y,
                          static_cast<int>(SYMB_WIDTH),
                          static_cast<int>(SYMB_HEIGHT)};

    SDL_SetRenderDrawColor(renderer, bg.red(), bg.green(), bg.blue(), 255);
    SDL_RenderFillRect(renderer, &bgRect);

    if (!m_pixelOffsets.empty()) {
        for (size_t i = 0; i < m_pixelOffsets.size(); ++i) {
            m_screenPoints[i].x = x + m_pixelOffsets[i].x;
            m_screenPoints[i].y = y + m_pixelOffsets[i].y;
        }

        SDL_SetRenderDrawColor(renderer, fg.red(), fg.green(), fg.blue(), 255);
        SDL_RenderDrawPoints(renderer, m_screenPoints.data(),
                             static_cast<int>(m_screenPoints.size()));
    }
}

// ============================================================================
// VGAFont — in-memory ROM data
// ============================================================================
void VGAFont::loadFromRomData(std::span<const uint8_t> romData) {
    const size_t numSymbols = std::min(romData.size() / SYMB_SIZE_BYTES,
                                       static_cast<size_t>(VGA_NUM_SYMBOLS));

    for (size_t i = 0; i < numSymbols; ++i) {
        const auto offset = i * SYMB_SIZE_BYTES;
        std::span<const uint8_t, SYMB_SIZE_BYTES> symbolBitmap(
            romData.data() + offset, SYMB_SIZE_BYTES);
        m_symbols[i].loadFromBitmap(symbolBitmap);
    }
}

namespace {

/// Parse a 2-char hex string ("7E") into a byte.
[[nodiscard]] constexpr uint8_t hexByte(const char* hex) noexcept {
    const auto nibble = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
        if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
        if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
        return 0;
    };
    return static_cast<uint8_t>((nibble(hex[0]) << 4) | nibble(hex[1]));
}

} // anonymous namespace

void VGAFont::loadFromMifFile(const char *path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error(std::format("Cannot open font file: '{}'", path));
    }

    const auto fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::string content(fileSize, '\0');
    file.read(content.data(), static_cast<std::streamsize>(fileSize));

    std::vector<uint8_t> data;
    data.reserve(VGA_NUM_SYMBOLS * SYMB_SIZE_BYTES);

    for (size_t i = 0; i < content.size(); ) {
        while (i < content.size() && (content[i] == '\n' || content[i] == '\r' || content[i] == ' ')) {
            ++i;
        }
        if (i >= content.size()) {
            break;
        }

        const size_t lineStart = i;
        while (i < content.size() && content[i] != '\n' && content[i] != '\r') {
            ++i;
        }
        if (i - lineStart != 2) {
            throw std::runtime_error(
                std::format("Invalid MIF line (expected 2 hex chars): '{}'", std::string_view(content.data() + lineStart, i - lineStart))
            );
        }

        data.push_back(hexByte(&content[lineStart]));
    }

    if (data.size() % SYMB_SIZE_BYTES != 0) {
        throw std::runtime_error(
            std::format("Invalid MIF file: total data size {} is not a multiple of symbol size {}",
                        data.size(), SYMB_SIZE_BYTES)
        );
    }

    loadFromRomData(data);
}

void VGAFont::loadFromPsfFile(const char *path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error(std::format("Cannot open font file: '{}'", path));
    }

    const auto fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::string content(fileSize, '\0');
    file.read(content.data(), static_cast<std::streamsize>(fileSize));

    // Binary with optional PSF header
    auto data = std::vector<uint8_t>(content.begin(), content.end());
    size_t offset = 0;

    if (fileSize >= 4) {
        if (static_cast<uint8_t>(content[0]) == 0x36 &&
            static_cast<uint8_t>(content[1]) == 0x04) {
            offset = 4;
        } else if (fileSize >= 32 &&
                   static_cast<uint8_t>(content[0]) == 0x72 &&
                   static_cast<uint8_t>(content[1]) == 0xB5 &&
                   static_cast<uint8_t>(content[2]) == 0x4A &&
                   static_cast<uint8_t>(content[3]) == 0x86) {
            offset = 32;
        }
    }

    if (offset > 0) {
        data.erase(data.begin(), data.begin() + static_cast<ptrdiff_t>(offset));
    }
    loadFromRomData(data);
}
