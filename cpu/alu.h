#ifndef ALU_H
#define ALU_H

#include <cstdint>
#include <string>

enum class ALUControl : uint8_t {
    ADD,    // suma con detección de overflow
    ADDU,   // suma sin detección de overflow
    SUB,    // resta con detección de overflow
    SUBU,   // resta sin detección de overflow
    AND,    // AND bit a bit
    OR,     // OR bit a bit
    XOR,    // XOR bit a bit
    SLT,    // setea a 1 si el primer operando es menor que el segundo 
    SLTU    // setea a 1 si el primer operado es menor que el segundo
};

struct ALUResult {
    uint32_t result;
    bool is_zero;
    bool overflow;
};

class ALU {
public:
    // ejecuta la operacion de la alu indicada por control sobre los operandos 
    static ALUResult execute(uint32_t operand_a, uint32_t operand_b, ALUControl control);

    // detecta overflow
    static bool detect_add_overflow(uint32_t a, uint32_t b, uint32_t result);
    static bool detect_sub_overflow(uint32_t a, uint32_t b, uint32_t result);

    // ALUControl a string
    static std::string control_to_string(ALUControl control);
};

#endif // ALU_H
