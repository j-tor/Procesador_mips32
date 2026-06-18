#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <array>
#include <cstddef>
#include <vector>
#include "Instruction.h"
#include "alu.h"

class CPU {
public:
    CPU();

    /// reset de la cpu
    void reset();

    // Stage-specific instruction cycle methods
    uint32_t fetch(const std::vector<uint32_t>& memory);

    // metodo para decodificar una instruccion
    Instruction decode(uint32_t instr);

    // metodos para ejecutar una instruccion
    void execute(Instruction instr);
    void execute(uint32_t instr_word);
    
    // execute specifically for R-type instructions
    ALUResult executeTypeR(Instruction instr);

    // writeback result to register
    void writeBack(size_t reg, uint32_t value);

    // execute a single instruction cycle step (Fetch, Decode, Execute, WriteBack, PC+=4)
    void step(const std::vector<uint32_t>& memory);

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

};

#endif // CPU_H
