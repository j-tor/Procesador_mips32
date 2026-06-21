#include "alu.h"


ALUResult ALU::execute(uint32_t operando_a, uint32_t operando_b, ALUControl control) {
    ALUResult res{};
    res.overflow = false;

    switch (control) {
        case ALUControl::ADD:
            res.result = operando_a + operando_b;
            res.overflow = detect_add_overflow(operando_a, operando_b, res.result);
            break;
        case ALUControl::ADDU:
            res.result = operando_a + operando_b;
            res.overflow = false;
            break;
        case ALUControl::SUB:
            res.result = operando_a - operando_b;
            res.overflow = detect_sub_overflow(operando_a, operando_b, res.result);
            break;
        case ALUControl::SUBU:
            res.result = operando_a - operando_b;
            res.overflow = false;
            break;
        case ALUControl::AND:
            res.result = operando_a & operando_b;
            break;
        case ALUControl::OR:
            res.result = operando_a | operando_b;
            break;
        case ALUControl::XOR:
            res.result = operando_a ^ operando_b;
            break;
        case ALUControl::NOR:
            res.result = ~(operando_a | operando_b);
            break;
        case ALUControl::SLT: {
            int32_t sa = static_cast<int32_t>(operando_a);
            int32_t sb = static_cast<int32_t>(operando_b);
            res.result = (sa < sb) ? 1 : 0;
            break;
        }
        case ALUControl::SLTU:
            res.result = (operando_a < operando_b) ? 1 : 0;
            break;
        default:
            res.result = 0;
            break;
    }

    res.is_zero = (res.result == 0);
    return res;
}

bool ALU::detect_add_overflow(uint32_t a, uint32_t b, uint32_t result) 
{
    int32_t sa = static_cast<int32_t>(a);
    int32_t sb = static_cast<int32_t>(b);
    int32_t sr = static_cast<int32_t>(result);

    // overflow si los signos de los operando son iguales, pero el del resultado es distinto
    return ((sa ^ sr) & (sb ^ sr)) < 0;
}

bool ALU::detect_sub_overflow(uint32_t a, uint32_t b, uint32_t result) 
{
    int32_t sa = static_cast<int32_t>(a);
    int32_t sb = static_cast<int32_t>(b);
    int32_t sr = static_cast<int32_t>(result);

    // overflow si los signos de los operando son distintos, y el del resultado difiere del signo de a
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
        case ALUControl::NOR:   
            return "NOR";
        case ALUControl::SLT:  
            return "SLT";
        case ALUControl::SLTU: 
            return "SLTU";
        default:               
            return "UNKNOWN";
    }
}
