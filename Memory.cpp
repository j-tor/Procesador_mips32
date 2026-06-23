#include "Memory.h"
#include "vga/VGAFramebuffer.h"
#include "vga/Keypad.h"
#include "Timer.h"

Memory::Memory(VGAFramebuffer& fb, Keypad& kp, Timer& tmr)
    : ram(RAM_SIZE, 0)
    , dmem(DM_SIZE, 0)
    , smem(STACK_SIZE, 0)
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

bool Memory::isDM(uint32_t addr) const {
    return addr >= DM_BASE && addr < DM_BASE + DM_SIZE;
}

bool Memory::isStack(uint32_t addr) const {
    return addr >= STACK_END - STACK_SIZE + 1 && addr <= STACK_END;
}

uint8_t Memory::readByte(uint32_t address) const {
    if (isKeypad(address))  {
        return (uint8_t)(keypad.state() & 0xFF);
    }
    if (isTimer(address)) {
        return (uint8_t)(timer.read() & 0xFF);
    }
    if (address < RAM_SIZE) {
        return ram[address];
    }
    if (isDM(address)) {
        return dmem[address - DM_BASE];
    }
    if (isStack(address)) {
        return smem[address - (STACK_END - STACK_SIZE + 1)];
    }
    return 0;
}

uint16_t Memory::readHalf(uint32_t address) const {
    if (isKeypad(address)) {
        return keypad.state();
    }
    if (isTimer(address)) {
        return (uint16_t)(timer.read() & 0xFFFF);
    }
    if (address + 1 < RAM_SIZE) {
        return (uint16_t)(ram[address]) |
               ((uint16_t)(ram[address + 1]) << 8);
    }
    if (isDM(address)) {
        uint32_t off = address - DM_BASE;
        return (uint16_t)(dmem[off]) |
               ((uint16_t)(dmem[off + 1]) << 8);
    }
    if (isStack(address)) {
        uint32_t off =address - (STACK_END - STACK_SIZE + 1);
        return (uint16_t)(smem[off]) |
               ((uint16_t)(smem[off + 1]) << 8);
    }
    return 0;
}

uint32_t Memory::readWord(uint32_t address) const {
    if (isKeypad(address)) {
        return keypad.state();
    }
    if (isTimer(address))  {
        return timer.read();
    }
    if (address + 3 < RAM_SIZE)  {
        return ((uint32_t)(ram[address + 3]) << 24) |
               ((uint32_t)(ram[address + 2]) << 16) |
               ((uint32_t)(ram[address + 1]) << 8)  |
               (uint32_t)(ram[address]);
    }
    if (isDM(address)) {
        uint32_t off= address - DM_BASE;
        return ((uint32_t)(dmem[off + 3]) << 24) |
               ((uint32_t)(dmem[off + 2]) << 16) |
               ((uint32_t)(dmem[off + 1]) << 8)  |
               (uint32_t)(dmem[off]);
    }
    if (isStack(address)) {
        uint32_t off = address - (STACK_END - STACK_SIZE + 1);
        return ((uint32_t)(smem[off + 3]) << 24) |
               ((uint32_t)(smem[off + 2]) << 16) |
               ((uint32_t)(smem[off + 1]) << 8)  |
               (uint32_t)(smem[off]);
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
            if (offset % 2 == 0)  {
                // escribe solo el caracter, leer el atributo actual de la celda
                vga.writePixel(row, col, value, 0x0F, 0x00);
            }
        }
        return;
    }
    if (address < RAM_SIZE) {
        ram[address] = value;
        return;
    }
    if (isDM(address)) {
        dmem[address - DM_BASE]= value;
        return;
    }
    if (isStack(address))  {
        smem[address - (STACK_END - STACK_SIZE + 1)] =value;
        return;
    }
}

void Memory::writeHalf(uint32_t address, uint16_t value) {
    if (isVGA(address)) {
        uint32_t offset = address - VGA_BASE;
        if (offset < 80 * 30 * 2 && offset % 2 == 0) {
            uint32_t cellIdx = offset / 2;
            size_t row = cellIdx / 80;
            size_t col = cellIdx % 80;
            uint8_t ch = value & 0xFF;
            uint8_t fg = (value >> 8) & 0x0F;
            uint8_t bg = (value >> 12) & 0x0F;
            vga.writePixel(row, col, ch, fg, bg);
        }
        return;
    }
    if (address + 1 < RAM_SIZE) {
        ram[address]  =(uint8_t)(value);
        ram[address + 1] =(uint8_t)(value >> 8);
        return;
    }
    if (isDM(address)) {
        uint32_t off =address - DM_BASE;
        dmem[off]  =(uint8_t)(value);
        dmem[off + 1] =(uint8_t)(value >> 8);
        return;
    }
    if (isStack(address)) {
        uint32_t off =address - (STACK_END - STACK_SIZE + 1);
        smem[off]  =(uint8_t)(value);
        smem[off + 1] =(uint8_t)(value >> 8);
        return;
    }
}

void Memory::writeWord(uint32_t address, uint32_t value) {
    if (isVGA(address)) {
        writeHalf(address, (uint16_t)(value & 0xFFFF));
        writeHalf(address + 2, (uint16_t)((value >> 16) & 0xFFFF));
        return;
    }
    if (address + 3 < RAM_SIZE) {
        ram[address]  =(uint8_t)(value);
        ram[address + 1] = (uint8_t)(value >> 8);
        ram[address + 2] = (uint8_t)(value >> 16);
        ram[address + 3] = (uint8_t)(value >> 24);
        return;
    }
    if (isDM(address)) {
        uint32_t off = address - DM_BASE;
        dmem[off] =(uint8_t)(value);
        dmem[off + 1]= (uint8_t)(value >> 8);
        dmem[off + 2]= (uint8_t)(value >> 16);
        dmem[off + 3] = (uint8_t)(value >> 24);
        return;
    }
    if (isStack(address)) {
        uint32_t off = address - (STACK_END - STACK_SIZE + 1);
        smem[off] =(uint8_t)(value);
        smem[off + 1] = (uint8_t)(value >> 8);
        smem[off + 2] = (uint8_t)(value >> 16);
        smem[off + 3] = (uint8_t)(value >> 24);
        return;
    }
}

void Memory::loadProgram(const std::vector<uint32_t>& prog, uint32_t address) {
    for (size_t i = 0; i < prog.size(); ++i)  {
        writeWord(address + (uint32_t)(i * 4), prog[i]);
    }
}

uint32_t Memory::fetchInstruction(uint32_t pc) const {
    return readWord(pc);
}
