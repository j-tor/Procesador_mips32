#include "VGATextWindow.h"

#include <format>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

VGATextWindow::VGATextWindow(int width, int height, Keypad& keypad) noexcept
    : m_width(width),
      m_height(height),
      m_keypad(keypad)
{}

bool VGATextWindow::initDisplay(VGAFramebuffer& fb,
                                std::span<const uint8_t> fontRom,
                                std::span<const uint8_t> paletteRom) {
    SDL_Window*   rawWindow   = nullptr;
    SDL_Renderer* rawRenderer = nullptr;

    const int res = SDL_CreateWindowAndRenderer(
        m_width, m_height, SDL_WINDOW_SHOWN, &rawWindow, &rawRenderer);

    if (res < 0) {
        std::cerr << std::format("Could not create window: '{}'\n", SDL_GetError());
        return false;
    }

    m_window.reset(rawWindow);
    m_renderer.reset(rawRenderer);
    SDL_SetWindowTitle(m_window.get(), "VGA Display 80×30");

    m_fb = &fb;
    m_palette.loadFromRomData(paletteRom);
    m_font.loadFromRomData(fontRom);

    return true;
}

bool VGATextWindow::initDisplay(VGAFramebuffer& fb,
                                const char* fontPath,
                                std::span<const uint8_t> paletteRom) {
    SDL_Window*   rawWindow   = nullptr;
    SDL_Renderer* rawRenderer = nullptr;

    const int res = SDL_CreateWindowAndRenderer(
        m_width, m_height, SDL_WINDOW_SHOWN, &rawWindow, &rawRenderer);

    if (res < 0) {
        std::cerr << std::format("Could not create window: '{}'\n", SDL_GetError());
        return false;
    }

    m_window.reset(rawWindow);
    m_renderer.reset(rawRenderer);
    SDL_SetWindowTitle(m_window.get(), "VGA Display 80×30");

    m_fb = &fb;
    m_palette.loadFromRomData(paletteRom);

    try {
        fs::path fontFile(fontPath);
        if (!fs::exists(fontFile)) {
            throw std::runtime_error(std::format("Font file does not exist: '{}'", fontPath));
        }
        std::string ext = fontFile.extension().string();

        if (ext == ".mif") {
            m_font.loadFromMifFile(fontPath);
        } else if (ext == ".psf") {
            m_font.loadFromPsfFile(fontPath);
        } else {
            throw std::runtime_error(std::format("Unsupported font file format: '{}'", ext));
        }
    } catch (const std::exception& ex) {
        std::cerr << std::format("Font load error: {}\n", ex.what());
        return false;
    }

    return true;
}

void VGATextWindow::drawVGASymbol(size_t row, size_t col, VGAFramebuffer::CellEntry symbolEntry) {
    const auto charIndex = static_cast<uint8_t>(symbolEntry & VGA_CELL_CHAR_MASK);
    const auto fgIndex   = static_cast<uint8_t>((symbolEntry & VGA_CELL_FG_MASK) >> VGA_CELL_FG_SHIFT);
    const auto bgIndex   = static_cast<uint8_t>((symbolEntry & VGA_CELL_BG_MASK) >> VGA_CELL_BG_SHIFT);

    const VGAColor& fg = m_palette[fgIndex];
    const VGAColor& bg = m_palette[bgIndex];
    const VGAFontSymbol& glyph = m_font[charIndex];

    glyph.draw(m_renderer.get(),
               col * static_cast<int>(SYMB_WIDTH),
               row * static_cast<int>(SYMB_HEIGHT),
               fg, bg);
}

void VGATextWindow::drawVGAContent() {
    if (m_font.size() == 0 || m_fb == nullptr) {
         return;
    }

    const VGAFramebuffer::CellBuffer& buffer = m_fb->swapAndGetFront();
    const VGAFramebuffer::CellEntry* fbData = buffer.data();

    for (size_t row = 0; (row < VGA_ROWS); ++row) {
        for (size_t col = 0; (col < VGA_COLS); ++col) {
            drawVGASymbol(row, col, *fbData++);
        }
    }
}

void VGATextWindow::redraw() {
    drawVGAContent();
    SDL_RenderPresent(m_renderer.get());
}

void VGATextWindow::postRepaintEvent() {
    SDL_Event event{};
    event.type            = SDL_WINDOWEVENT;
    event.window.event    = SDL_WINDOWEVENT_EXPOSED;
    event.window.windowID = SDL_GetWindowID(m_window.get());
    SDL_PushEvent(&event);
}

void VGATextWindow::saveScreenshot(const char* filename) {
        SDLSurfacePtr screenshot{
        SDL_CreateRGBSurface(0, m_width, m_height, 32,
            0x00FF0000,
            0x0000FF00,
            0x000000FF,
            0xFF000000
        )
    };

    if (!screenshot) {
        std::cerr << std::format("CreateRGBSurface failed: '{}'\n", SDL_GetError());
        return;
    }

    if (SDL_RenderReadPixels(m_renderer.get(), nullptr,
                             SDL_GetWindowPixelFormat(m_window.get()),
                             screenshot->pixels, screenshot->pitch) != 0) {
        std::cerr << std::format("RenderReadPixels failed: '{}'\n", SDL_GetError());
        return;
    }

    SDL_SaveBMP(screenshot.get(), filename);
}

void VGATextWindow::handleKeyDown(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT:  m_keypad.pressKey(0);   break;
        case SDLK_RIGHT: m_keypad.pressKey(1);   break;
        case SDLK_DOWN:  m_keypad.pressKey(2);   break;
        case SDLK_UP:    m_keypad.pressKey(3);   break;
        case SDLK_q:     m_keypad.pressKey(4);   break;
        case SDLK_p:     m_keypad.pressKey(5);   break;
        case SDLK_b:     m_keypad.pressKey(6);   break;
        case SDLK_SPACE: m_keypad.pressKey(7);   break;
        case SDLK_PAGEUP:   m_keypad.pressKey(8);   break;
        case SDLK_PAGEDOWN: m_keypad.pressKey(9);   break;
        case SDLK_HOME:     m_keypad.pressKey(10);  break;
        case SDLK_END:      m_keypad.pressKey(11);  break;
        case SDLK_INSERT:   m_keypad.pressKey(12);  break;
        case SDLK_DELETE:   m_keypad.pressKey(13);  break;
        case SDLK_F1:       m_keypad.pressKey(14);  break;
        case SDLK_F2:       m_keypad.pressKey(15);  break;
        case SDLK_r:     m_keypad.pressReset();   break;
        case SDLK_s:     m_keypad.pressStop();    break;
        default: break;
    }
}

void VGATextWindow::handleKeyUp(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT:  m_keypad.releaseKey(0);   break;
        case SDLK_RIGHT: m_keypad.releaseKey(1);   break;
        case SDLK_DOWN:  m_keypad.releaseKey(2);   break;
        case SDLK_UP:    m_keypad.releaseKey(3);   break;
        case SDLK_q:     m_keypad.releaseKey(4);   break;
        case SDLK_p:     m_keypad.releaseKey(5);   break;
        case SDLK_b:     m_keypad.releaseKey(6);   break;
        case SDLK_SPACE: m_keypad.releaseKey(7);   break;
        case SDLK_PAGEUP:   m_keypad.releaseKey(8);   break;
        case SDLK_PAGEDOWN: m_keypad.releaseKey(9);   break;
        case SDLK_HOME:     m_keypad.releaseKey(10);  break;
        case SDLK_END:      m_keypad.releaseKey(11);  break;
        case SDLK_INSERT:   m_keypad.releaseKey(12);  break;
        case SDLK_DELETE:   m_keypad.releaseKey(13);  break;
        case SDLK_F1:       m_keypad.releaseKey(14);  break;
        case SDLK_F2:       m_keypad.releaseKey(15);  break;
        case SDLK_r:     m_keypad.releaseReset();   break;
        case SDLK_s:     m_keypad.releaseStop();    break;
        default: break;
    }
}

void VGATextWindow::run() {
    SDL_Event event;
    m_quitRequested.store(false, std::memory_order_relaxed);

    while (!m_quitRequested.load(std::memory_order_relaxed)) {
        SDL_WaitEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                m_quitRequested.store(true, std::memory_order_relaxed);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_EXPOSED) redraw();
                break;
            case SDL_KEYDOWN:
                handleKeyDown(event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                handleKeyUp(event.key.keysym.sym);
                break;
            default: break;
        }
    }
}

void VGATextWindow::poll() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                m_quitRequested.store(true, std::memory_order_relaxed);
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_EXPOSED) redraw();
                break;
            case SDL_KEYDOWN:
                handleKeyDown(event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                handleKeyUp(event.key.keysym.sym);
                break;
            default: break;
        }
    }
}
