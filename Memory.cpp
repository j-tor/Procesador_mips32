#include "Memory.h"
#include "vga/VGAFramebuffer.h"
#include "vga/Keypad.h"
#include "Timer.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

Memory::Memory(VGAFramebuffer& fb, Keypad& kp, Timer& tmr)
    : ram(RAM_SIZE, 0)
    , vga(fb)
    , keypad(kp)
    , timer(tmr)
{}

bool Memory::isVGA(uint32_t addr) const {
    return addr >= VGA_BASE && addr <= VGA_END;
}

bool Memory::isKeypad(uint32_t addr) const {
    return addr == KEYPAD_ADDR;
}

bool Memory::isTimer(uint32_t addr) const {
    return addr == TIMER_ADDR;
}

uint8_t Memory::readByte(uint32_t address) const {
    if (isKeypad(address)) {
        return static_cast<uint8_t>(keypad.state() & 0xFF);
    }
    if (isTimer(address)) {
        return static_cast<uint8_t>(timer.read() & 0xFF);
    }
    if (address < RAM_SIZE) {
        return ram[address];
    }
    return 0;
}

uint16_t Memory::readHalf(uint32_t address) const {
    if (isKeypad(address)) {
        return keypad.state();
    }
    if (isTimer(address)) {
        return timer.read();
    }
    if (address + 1 < RAM_SIZE) {
        uint16_t low = ram[address];
        uint16_t high = ram[address + 1];
        return (high << 8) | low;
    }
    return 0;
}

uint32_t Memory::readWord(uint32_t address) const {
    if (isKeypad(address)) {
        return keypad.state();
    }
    if (isTimer(address)) {
        return timer.read();
    }
    if (address + 3 < RAM_SIZE) {
        return (static_cast<uint32_t>(ram[address + 3]) << 24) |
               (static_cast<uint32_t>(ram[address + 2]) << 16) |
               (static_cast<uint32_t>(ram[address + 1]) << 8)  |
               static_cast<uint32_t>(ram[address]);
    }
    return 0;
}

void Memory::writeByte(uint32_t address, uint8_t value) {
    if (isVGA(address)) {
        uint32_t offset = address - VGA_BASE;
        if (offset < 80 * 30 * 2) {
            uint32_t cellIdx = offset / 2;
            size_t row = cellIdx / 80;
            size_t col = cellIdx % 80;
            if (offset % 2 == 0) {
                // lee el valor actual de 16 bits
                uint16_t currentCell = readHalf(address);
                // actualiza solo los 8 bits bajos 
                uint16_t newCell = (currentCell & 0xFF00) | value;

                // separrado en variables simples para la pantalla
                uint8_t character = newCell & 0xFF;
                uint8_t fgColor = (newCell >> 8) & 0x0F;
                uint8_t bgColor = (newCell >> 12) & 0x0F;

                vga.writePixel(row, col, character, fgColor, bgColor);
            }
        }
        return;
    }
    if (address < RAM_SIZE) {
        ram[address] = value;
    }
}

void Memory::writeHalf(uint32_t address, uint16_t value) {
    if (isVGA(address)) {
        uint32_t offset = address - VGA_BASE;
        if (offset < 80 * 30 * 2 && offset % 2 == 0) {
            uint32_t cellIdx = offset / 2;
            size_t row = cellIdx / 80;
            size_t col = cellIdx % 80;

            // separado de variables para la pantalla
            uint8_t character = value & 0xFF;
            uint8_t fgColor = (value >> 8) & 0x0F;
            uint8_t bgColor = (value >> 12) & 0x0F;

            vga.writePixel(row, col, character, fgColor, bgColor);
        }
        return;
    }
    if (address + 1 < RAM_SIZE) {
        ram[address]     = value & 0xFF;
        ram[address + 1] = (value >> 8) & 0xFF;
    }
}

void Memory::writeWord(uint32_t address, uint32_t value) {
    if (isVGA(address)) {
        writeHalf(address, static_cast<uint16_t>(value & 0xFFFF));
        writeHalf(address + 2, static_cast<uint16_t>((value >> 16) & 0xFFFF));
        return;
    }
    if (address + 3 < RAM_SIZE) {
        ram[address]     = static_cast<uint8_t>(value & 0xFF);
        ram[address + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        ram[address + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        ram[address + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    }
}

void Memory::loadProgram(const std::vector<uint32_t>& instruction, uint32_t address) {
    for (size_t i = 0; i < instruction.size(); ++i) {
        writeWord(address + static_cast<uint32_t>(i * 4), instruction[i]);
    }
}

void Memory::loadData(const std::vector<uint8_t>& data, uint32_t address) {
    for (size_t i = 0; i < data.size(); ++i) {
        if (address + i < RAM_SIZE) {
            ram[address + i] = data[i];
        }
    }
}

uint32_t Memory::fetchInstruction(uint32_t pc) const {
    return readWord(pc);
}
