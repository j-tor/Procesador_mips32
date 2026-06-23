/**
 * VGATextWindow — Standalone Test / Demo
 *
 * Tests the VGA text-mode display with a font file, three demo patterns,
 * and a background timer thread that updates a seconds counter on the screen.
 *
 * Usage:
 *   vga_test <font_file>
 *
 * Font formats (auto-detected): MIF hex-per-line, PSF v1/v2, raw binary.
 */

#include "vga/VGATextWindow.h"
#include "vga/VGAFramebuffer.h"
#include "vga/VGAFont.h"
#include "vga/Keypad.h"
#include "Timer.h"
#include "Recursos/CliArgs.hpp"
#include "file_loader.h"
#include "Memory.h"
#include "cpu/cpu.h"

#include <array>
#include <atomic>
#include <fstream>
#include <cstdint>
#include <format>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

// ============================================================================
// Standard VGA 16-color palette (16 bytes of 4-bit VGA color codes)
// ============================================================================
inline constexpr std::array<uint8_t, 16> kDefaultVGAPalette = {
    0x00,  // 0: Black
    0x01,  // 1: Blue
    0x02,  // 2: Green
    0x03,  // 3: Cyan
    0x04,  // 4: Red
    0x05,  // 5: Magenta
    0x06,  // 6: Brown/Orange
    0x07,  // 7: Light Gray
    0x08,  // 8: Dark Gray
    0x09,  // 9: Light Blue
    0x0A,  // 10: Light Green
    0x0B,  // 11: Light Cyan
    0x0C,  // 12: Light Red
    0x0D,  // 13: Light Magenta
    0x0E,  // 14: Yellow
    0x0F,  // 15: White
};

// ============================================================================
// Demo patterns
// ============================================================================

void fillAsciiTable(VGAFramebuffer& fb) {
    for (size_t row = 0; row < VGA_ROWS; ++row) {
        const auto fg = static_cast<uint8_t>((row % 14) + 1);
        for (size_t col = 0; col < VGA_COLS; ++col) {
            const char c = static_cast<char>(0x20 + ((row * VGA_COLS + col) % 96));
            fb.writePixel(row, col, c, fg, 0x00);
        }
    }

    fb.commitFrame();
}

void fillColorBars(VGAFramebuffer& fb) {
    for (size_t row = 0; row < VGA_ROWS; ++row) {
        const auto bg = static_cast<uint8_t>(row % 16);
        for (size_t col = 0; col < VGA_COLS; ++col) {
            const auto fg = static_cast<uint8_t>(col % 16);
            fb.writePixel(row, col, '#', fg, bg);
        }
    }

    fb.commitFrame();
}

void fillHelloWorld(VGAFramebuffer& fb) {
    fb.clear(0x07, 0x00);

    constexpr const char* message = "Hello, Organizacion de Computadoras!";
    constexpr size_t msgLen = 35;  // length of above string
    constexpr size_t startRow = 14;
    constexpr size_t startCol = (VGA_COLS - msgLen) / 2;

    for (size_t i = 0; i < msgLen; ++i) {
        const auto fg = static_cast<uint8_t>(1 + (i % 15));
        fb.writePixel(startRow, startCol + i, message[i], fg, 0x0F);
    }
    fb.commitFrame();
}

int main(int argc, char* argv[]) {
    CliArgs args(argc, (const char**)argv);

    // Revisar si hubo error en los argumentos
    if (args.parse()) {
        std::cout << "Error en los argumentos de consola.\n";
        return 1;
    }

    // Vector para guardar la memoria
    std::vector<uint32_t> memoriaInstrucciones;

    try {
        // Cargar el archivo
        Fileloader loader(args.programPath());
        memoriaInstrucciones = loader.getInstructiones();

        std::cout << "Archivo cargado correctamente.\n";
    } catch (std::exception& e) {
        // Mostrar el error si algo falla
        std::cout << e.what() << "\n";
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << std::format("SDL_Init failed: '{}'\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    struct SDLGuard { ~SDLGuard() { SDL_Quit(); } } sdlGuard;

    Keypad         keypad;
    VGAFramebuffer framebuffer;
    Timer          timer;
    Memory         memory(framebuffer, keypad, timer);
    CPU            cpu;

    memory.loadProgram(memoriaInstrucciones, Memory::IM_BASE);
    cpu.setMemory(&memory);
    cpu.setPC(Memory::IM_BASE);

    try {
        std::ifstream dataFile(args.dataPath(), std::ios::binary | std::ios::ate);
        if (!dataFile.is_open())  
        {
            throw std::runtime_error("no se pudo abrir " + args.dataPath());
        }
        std::streamsize dataSize = dataFile.tellg();
        dataFile.seekg(0, std::ios::beg);
        std::vector<uint8_t> dataBytes(static_cast<size_t>(dataSize));
        if (!dataFile.read(reinterpret_cast<char*>(dataBytes.data()), dataSize)) 
        {
            throw std::runtime_error("Error al leer " + args.dataPath());
        }
        uint32_t dataAddr = (memoriaInstrucciones.size() * 4 + 0xFF) & ~0xFF;
        memory.loadData(dataBytes, dataAddr);
        std::cout << "datos cargados (" << dataBytes.size() << " bytes en 0x" << std::hex << dataAddr << std::dec << ").\n";
    } catch (std::exception& e) {
        std::cout << "advertencia: " << e.what() << " — continuando sin datos.\n";
    }

    fillHelloWorld(framebuffer);

    VGATextWindow window(VGA_WINDOW_WIDTH, VGA_WINDOW_HEIGHT, keypad);

    // Load font from file (format auto-detected by VGAFont)
    if (!window.initDisplay(framebuffer, args.fontPath().c_str(), kDefaultVGAPalette)) {
        std::cerr << "Failed to initialize VGA display.\n";
        return EXIT_FAILURE;
    }

    window.redraw();

    std::atomic<bool> stopTimerThread{false};
    std::atomic<bool> needsCommitFrame{false};
    std::atomic<bool> runningMode{false};

    auto timerFuture = std::async(std::launch::async, [&]() {
        uint32_t lastDisplayedSecond = 0;

        while (!stopTimerThread.load(std::memory_order_acquire)) {
            const uint32_t nowMs   = timer.read();
            const uint32_t seconds = nowMs / 1000;

            if (seconds != lastDisplayedSecond) {
                lastDisplayedSecond = seconds;

                char buf[64];
                std::snprintf(buf, sizeof(buf),
                              "Timer: %4us  (ms = %5u)", seconds, nowMs);
                framebuffer.writeString(0, 2, buf, 0x0A, 0x00);  // green on black
                needsCommitFrame.store(true, std::memory_order_release);
                window.postRepaintEvent();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    auto cpuFuture = std::async(std::launch::async, [&]() {
        while (!stopTimerThread.load(std::memory_order_acquire)) {
            if (runningMode.load(std::memory_order_acquire) && !cpu.isStopped()) {
                for (int i = 0; i < 500000; ++i) {
                    if (!runningMode.load(std::memory_order_acquire))  {
                        break;
                    }
                    cpu.step();
                    if (cpu.isStopped()) break;
                }
                needsCommitFrame.store(true, std::memory_order_release);
                window.postRepaintEvent();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        }
    });

    std::cout << std::format(
        "VGA Text Window Test\n"
        "  Resolution: {}×{} ({} cols × {} rows)\n"
        "  Font: 8×16 bitmap\n"
        "  Press SPACE to cycle demo patterns, Q to quit\n"
        "  [R] Reset CPU    [S] Stop/Resume CPU\n"
        "  Timer thread: updates row 0 every second\n",
        VGA_WINDOW_WIDTH, VGA_WINDOW_HEIGHT, VGA_COLS, VGA_ROWS);

    int currentPattern = 0;
    SDL_Event event;
    bool running = true;

    while (running && !window.quitRequested()) {
        SDL_WaitEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
                    if (needsCommitFrame.load(std::memory_order_acquire)) {
                        framebuffer.commitFrame();
                        needsCommitFrame.store(false, std::memory_order_release);
                    }
                    window.redraw();
                }
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        running = false;
                        break;
                    case SDLK_SPACE:
                        currentPattern = (currentPattern + 1) % 3;
                        switch (currentPattern) {
                            case 0: fillHelloWorld(framebuffer); break;
                            case 1: fillAsciiTable(framebuffer); break;
                            case 2: fillColorBars(framebuffer);  break;
                        }
                        window.redraw();
                        break;
                    case SDLK_r:
                        runningMode.store(false, std::memory_order_release);
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                        keypad.pressReset();
                        cpu.reset();
                        cpu.setMemory(&memory);
                        cpu.setPC(Memory::IM_BASE);
                        framebuffer.clear(0x07, 0x00);
                        framebuffer.commitFrame();
                        window.redraw();
                        std::cout << "[SIM] Reset\n";
                        break;
                    case SDLK_s:
                        runningMode = !runningMode;
                        keypad.pressStop();
                        std::cout << (runningMode ? "[SIM] Running\n" : "[SIM] Stopped\n");
                        break;
                    default:
                        switch (event.key.keysym.sym) {
                            case SDLK_LEFT:  keypad.pressKey(0); break;
                            case SDLK_RIGHT: keypad.pressKey(1); break;
                            case SDLK_DOWN:  keypad.pressKey(2); break;
                            case SDLK_UP:    keypad.pressKey(3); break;
                            default: break;
                        }
                        break;
                }
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:  keypad.releaseKey(0); break;
                    case SDLK_RIGHT: keypad.releaseKey(1); break;
                    case SDLK_DOWN:  keypad.releaseKey(2); break;
                    case SDLK_UP:    keypad.releaseKey(3); break;
                    case SDLK_r: keypad.releaseReset(); break;
                    case SDLK_s: keypad.releaseStop();  break;
                    default: break;
                }
                break;

            default: break;
        }
    }

    stopTimerThread.store(true, std::memory_order_release);
    timerFuture.wait();
    cpuFuture.wait();

    std::cout << std::format("VGA test finished. Final timer: {} ms\n", timer.read());
    return EXIT_SUCCESS;
}
