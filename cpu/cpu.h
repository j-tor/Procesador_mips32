#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <array>
#include <cstddef>
#include <vector>
#include "Instruction.h"
#include "alu.h"

class Memory;

class CPU {
public:
    CPU();

    /// reset de la cpu
    void reset();

    void setMemory(Memory* mem) { m_memory = mem; }

    // metodo para decodificar  instruccion
    Instruction decode(uint32_t instr);

    // metodo para ejecutar instruccion
    void execute(Instruction instr);
    void execute(uint32_t instr_word);
    
    // metodo para instrucciones tipo R
    ALUResult executeTypeR(Instruction instr);

    void writeBack(size_t reg, uint32_t value);

    void step();

    // metodo para el PC
    uint32_t getPC() const;
    void setPC(uint32_t newPC);

    // metodos para los registros
    uint32_t readRegister(size_t reg) const;
    void writeRegister(size_t reg, uint32_t value);

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
    Memory* m_memory{nullptr};

};

#endif // CPU_H
