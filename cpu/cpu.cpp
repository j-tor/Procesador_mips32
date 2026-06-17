#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "cpu.h"

CPU::CPU() {
    reset();
}

void CPU::reset() {
    PC = 0;
    regs.fill(0);
    HI = 0;
    LO = 0;
    stopped = false;
}

uint32_t CPU::readRegister(size_t reg) const {
    if (reg >= regs.size()) {
        throw std::out_of_range(std::format("Register index {} out of range (0-31)", reg));
    }
    if (reg == 0) {
        return 0;
    }
    return regs[reg];
}

void CPU::writeRegister(size_t reg, uint32_t value) {
    if (reg >= regs.size()) {
        throw std::out_of_range(std::format("Register index {} out of range (0-31)", reg));
    }
    if (reg != 0) {
        regs[reg] = value;
    }
}

uint32_t CPU::getPC() const {
    return PC;
}

void CPU::setPC(uint32_t newPC) {
    PC = newPC;
}

uint32_t CPU::getHI() const {
    return HI;
}

void CPU::setHI(uint32_t value) {
    HI = value;
}

uint32_t CPU::getLO() const {
    return LO;
}

void CPU::setLO(uint32_t value) {
    LO = value;
}

bool CPU::isStopped() const {
    return stopped;
}

void CPU::setStopped(bool stop) {
    stopped = stop;
}

Instruction CPU::decode(uint32_t instr) {
    Instruction decoded;
    decoded.raw = instr;

    uint32_t opcode = decoded.opcode;
    uint32_t funct = decoded.funct;
    std::string instrName = "";

    switch (opcode) {
        case 0x00: { // Tipo R
            switch (funct) {
                case 0x00: instrName = "SLL"; break;
                case 0x02: instrName = "SRL"; break;
                case 0x03: instrName = "SRA"; break;
                case 0x04: instrName = "SLLV"; break;
                case 0x06: instrName = "SRLV"; break;
                case 0x07: instrName = "SRAV"; break;
                case 0x08: instrName = "JR"; break;
                case 0x09: instrName = "JALR"; break;
                case 0x0C: instrName = "SYSCALL"; break;
                case 0x0D: instrName = "BREAK"; break;
                case 0x10: instrName = "MFHI"; break;
                case 0x11: instrName = "MTHI"; break;
                case 0x12: instrName = "MFLO"; break;
                case 0x13: instrName = "MTLO"; break;
                case 0x18: instrName = "MULT"; break;
                case 0x19: instrName = "MULTU"; break;
                case 0x1A: instrName = "DIV"; break;
                case 0x1B: instrName = "DIVU"; break;
                case 0x20: instrName = "ADD"; break;
                case 0x21: instrName = "ADDU"; break;
                case 0x22: instrName = "SUB"; break;
                case 0x23: instrName = "SUBU"; break;
                case 0x24: instrName = "AND"; break;
                case 0x25: instrName = "OR"; break;
                case 0x26: instrName = "XOR"; break;
                case 0x27: instrName = "NOR"; break;
                case 0x2A: instrName = "SLT"; break;
                case 0x2B: instrName = "SLTU"; break;
                default:
                    throw std::invalid_argument(std::format("Instrucción Tipo-R no reconocida con funct: 0x{:02X}", funct));
            }
            break;
        }
        case 0x02: instrName = "J"; break;
        case 0x03: instrName = "JAL"; break;
        case 0x04: instrName = "BEQ"; break;
        case 0x05: instrName = "BNE"; break;
        case 0x06: instrName = "BLEZ"; break;
        case 0x07: instrName = "BGTZ"; break;
        case 0x08: instrName = "ADDI"; break;
        case 0x09: instrName = "ADDIU"; break;
        case 0x0A: instrName = "SLTI"; break;
        case 0x0B: instrName = "SLTIU"; break;
        case 0x0C: instrName = "ANDI"; break;
        case 0x0D: instrName = "ORI"; break;
        case 0x0E: instrName = "XORI"; break;
        case 0x0F: instrName = "LUI"; break;
        case 0x20: instrName = "LB"; break;
        case 0x21: instrName = "LH"; break;
        case 0x23: instrName = "LW"; break;
        case 0x24: instrName = "LBU"; break;
        case 0x25: instrName = "LHU"; break;
        case 0x28: instrName = "SB"; break;
        case 0x29: instrName = "SH"; break;
        case 0x2B: instrName = "SW"; break;
        default:
            throw std::invalid_argument(std::format("Instrucción no reconocida con opcode: 0x{:02X}", opcode));
    }

    std::cout << std::format("[Decoder] Instrucción identificada: {} (raw: 0x{:08X})\n", instrName, instr);
    return decoded;
}
