#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>

union Instruction {
    uint32_t raw;

    // formato R
    struct {
        uint32_t funct  : 6;
        uint32_t shamt  : 5;
        uint32_t rd     : 5;
        uint32_t rt     : 5;
        uint32_t rs     : 5;
        uint32_t opcode : 6;
    };

    // formato i (funct, shamt, rd)
    struct {
        uint32_t imm    : 16;
    };

    // formato j (funct, shamt, rd, rt, rs)
    struct {
        uint32_t target : 26;
    };
};

#endif // INSTRUCTION_H
