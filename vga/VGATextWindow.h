#pragma once

#include <cstdint>
#include <atomic>
#include <memory>
#include <optional>
#include <span>
#include "VGAConstants.h"
#include "VGAColor.h"
#include "VGAFont.h"
#include "VGAFramebuffer.h"
#include "Keypad.h"

#include <SDL.h>

// ============================================================================
// SDL Resource Deleters (RAII via unique_ptr)
// ============================================================================
struct SDLWindowDeleter {
    void operator()(SDL_Window* w) const noexcept { if (w) SDL_DestroyWindow(w); }
};

struct SDLRendererDeleter {
    void operator()(SDL_Renderer* r) const noexcept { if (r) SDL_DestroyRenderer(r); }
};

struct SDLSurfaceDeleter {
    void operator()(SDL_Surface* s) const noexcept { if (s) SDL_FreeSurface(s); }
};

using SDLSurfacePtr = std::unique_ptr<SDL_Surface, SDLSurfaceDeleter>;

class VGATextWindow {
public:
    explicit VGATextWindow(int width, int height, Keypad& keypad) noexcept;

    ~VGATextWindow() = default;

    // Non-copyable, non-movable
    VGATextWindow(const VGATextWindow&)            = delete;
    VGATextWindow& operator=(const VGATextWindow&) = delete;
    VGATextWindow(VGATextWindow&&)                 = delete;
    VGATextWindow& operator=(VGATextWindow&&)      = delete;

    [[nodiscard]] bool initDisplay(VGAFramebuffer& fb,
                                   std::span<const uint8_t> fontRom,
                                   std::span<const uint8_t> paletteRom);

    [[nodiscard]] bool initDisplay(VGAFramebuffer& fb,
                                   const char* fontPath,
                                   std::span<const uint8_t> paletteRom);

    void run();
    void poll();

    void redraw();
    void postRepaintEvent();

    void saveScreenshot(const char* filename);

    [[nodiscard]] int  width()  const noexcept
    { return m_width; }

    [[nodiscard]] int  height() const noexcept
    { return m_height; }

    [[nodiscard]] bool quitRequested() const noexcept
    { return m_quitRequested.load(std::memory_order_acquire); }

private:
    void drawVGAContent();
    void drawVGASymbol(size_t row, size_t col, VGAFramebuffer::CellEntry symbolEntry);
    void handleKeyDown(SDL_Keycode key);
    void handleKeyUp(SDL_Keycode key);

private:
    int m_width;
    int m_height;

    // External keypad — we write SDL key events here; SoC reads from it
    Keypad& m_keypad;

    // SDL resources
    std::unique_ptr<SDL_Window,   SDLWindowDeleter>   m_window;
    std::unique_ptr<SDL_Renderer, SDLRendererDeleter> m_renderer;

    // VGA state
    VGAColorPalette  m_palette;
    VGAFont          m_font;
    VGAFramebuffer*  m_fb{nullptr};

    // Repaint & quit (atomic for cross-thread access)
    std::atomic<bool> m_quitRequested {false};
};
