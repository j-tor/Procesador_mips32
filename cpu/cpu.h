#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <array>
#include <cstddef>

class CPU {
public:
    CPU();

    /// reset de la cpu
    void reset();

    // metodo para el PC
    uint32_t getPC() const;
    void setPC(uint32_t newPC);

    // metodos para HI, LO y stopped
    uint32_t getHI() const;
    void setHI(uint32_t value);

    uint32_t getLO() const;
    void setLO(uint32_t value);
    bool isStopped() const;
    void setStopped(bool stop);

private:
    uint32_t PC{0};
    std::array<uint32_t, 32> regs{};
    uint32_t HI{0};
    uint32_t LO{0};
    bool stopped{false};

};

#endif // CPU_H
