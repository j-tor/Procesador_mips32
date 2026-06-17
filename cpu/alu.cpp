#include "alu.h"


bool ALU::detect_add_overflow(uint32_t a, uint32_t b, uint32_t result) 
{
    int32_t sa = static_cast<int32_t>(a);
    int32_t sb = static_cast<int32_t>(b);
    int32_t sr = static_cast<int32_t>(result);

    // overflow si los signos de los operandos son iguales, pero el del resultado es distinto
    return ((sa ^ sr) & (sb ^ sr)) < 0;
}

bool ALU::detect_sub_overflow(uint32_t a, uint32_t b, uint32_t result) 
{
    int32_t sa = static_cast<int32_t>(a);
    int32_t sb = static_cast<int32_t>(b);
    int32_t sr = static_cast<int32_t>(result);

    // overflow si los signos de los operandos son distintos, y el del resultado difiere del signo de a
    return ((sa ^ sb) & (sa ^ sr)) < 0;
}

std::string ALU::control_to_string(ALUControl control) 
{
    switch (control) {
        case ALUControl::ADD:  
            return "ADD";
        case ALUControl::ADDU: 
            return "ADDU";
        case ALUControl::SUB:  
            return "SUB";
        case ALUControl::SUBU: 
            return "SUBU";
        case ALUControl::AND:  
            return "AND";
        case ALUControl::OR:   
            return "OR";
        case ALUControl::XOR:  
            return "XOR";
        case ALUControl::SLT:  
            return "SLT";
        case ALUControl::SLTU: 
            return "SLTU";
        default:               
            return "UNKNOWN";
    }
}
