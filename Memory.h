#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <vector>

class VGAFramebuffer;
class Keypad;
class Timer;

class Memory {
public:
    static constexpr uint32_t RAM_SIZE   = 1024 * 1024;
    static constexpr uint32_t DM_SIZE    = 8 * 1024;
    static constexpr uint32_t STACK_SIZE = 8 * 1024;
    static constexpr uint32_t VGA_BASE   = 0x0000B800;
    static constexpr uint32_t VGA_END    = VGA_BASE + 80 * 30 * 2 - 1;
    static constexpr uint32_t KEYPAD_ADDR = 0xFFFF0004;
    static constexpr uint32_t TIMER_ADDR  = 0xFFFF0008;
    static constexpr uint32_t IM_BASE    = 0x00004000;
    static constexpr uint32_t DM_BASE    = 0x10000000;
    static constexpr uint32_t STACK_END  = 0x7FFFEFFC;

    Memory(VGAFramebuffer& fb, Keypad& kp, Timer& tmr);

    uint8_t  readByte(uint32_t address) const;
    uint16_t readHalf(uint32_t address) const;
    uint32_t readWord(uint32_t address) const;

    void writeByte(uint32_t address, uint8_t value);
    void writeHalf(uint32_t address, uint16_t value);
    void writeWord(uint32_t address, uint32_t value);

    void loadProgram(const std::vector<uint32_t>& prog, uint32_t address);
    uint32_t fetchInstruction(uint32_t pc) const;

private:
    std::vector<uint8_t> ram;     //  memoria principal
    std::vector<uint8_t> dmem;    // memoria  de datos 
    std::vector<uint8_t> smem;    //pila 

    VGAFramebuffer& vga;
    Keypad& keypad;
    Timer& timer;

    bool isVGA(uint32_t addr) const;
    bool isKeypad(uint32_t addr) const;
    bool isTimer(uint32_t addr) const;
    bool isDM(uint32_t addr) const;
    bool isStack(uint32_t addr) const;
};

#endif
