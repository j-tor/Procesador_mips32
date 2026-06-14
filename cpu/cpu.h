#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <array>

class CPU {
public:
    uint32_t PC{0};
    std::array<uint32_t, 32> regs{};
    uint32_t HI{0};
    uint32_t LO{0};
    bool stopped{false};

    CPU() noexcept = default;

    /// reset de la cpu
    void reset() noexcept {
        PC = 0;
        regs.fill(0);
        HI = 0;
        LO = 0;
        stopped = false;
    }
};

#endif // CPU_H
