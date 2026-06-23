#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "cpu.h"
#include "Memory.h"

CPU::CPU() {
    reset();
}

void CPU::reset() {
    PC = 0;
    regs.fill(0);
    regs[29] = 0x7FFFEFFC; 
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
        case 0x00: { // tipo R
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

void CPU::execute(Instruction instr) {
    uint32_t opcode = instr.opcode;
    uint32_t funct = instr.funct;
    uint32_t nextPC = PC + 4;

    int32_t simm = static_cast<int16_t>(instr.imm);
    uint32_t uimm = instr.imm;

    switch (opcode) {
        case 0x00: { // tipo R
            switch (funct) {
                // ALU operando
                case 0x20: // ADD
                case 0x21: // ADDU
                case 0x22: // SUB
                case 0x23: // SUBU
                case 0x24: // AND
                case 0x25: // OR
                case 0x26: // XOR
                case 0x27: // NOR
                case 0x2A: // SLT
                case 0x2B: { // SLTU
                    ALUResult res = executeTypeR(instr);
                    if (!(res.overflow && (funct == 0x20 || funct == 0x22))) {
                        writeRegister(instr.rd, res.result);
                    }
                    break;
                }
                // Shifts y funciones tipo R
                case 0x00: 
                    writeRegister(instr.rd, readRegister(instr.rt) << instr.shamt); break; // SLL
                case 0x02: 
                    writeRegister(instr.rd, readRegister(instr.rt) >> instr.shamt); break; // SRL
                case 0x03: { // SRA
                    int32_t rt_val = static_cast<int32_t>(readRegister(instr.rt));
                    writeRegister(instr.rd, static_cast<uint32_t>(rt_val >> instr.shamt));
                    break;
                }
                case 0x04: 
                    writeRegister(instr.rd, readRegister(instr.rt) << (readRegister(instr.rs) & 0x1F)); break; // SLLV
                case 0x06: 
                    writeRegister(instr.rd, readRegister(instr.rt) >> (readRegister(instr.rs) & 0x1F)); break; // SRLV
                case 0x07: { // SRAV
                    int32_t rt_val = static_cast<int32_t>(readRegister(instr.rt));
                    writeRegister(instr.rd, static_cast<uint32_t>(rt_val >> (readRegister(instr.rs) & 0x1F)));
                    break;
                }
                case 0x08: 
                    nextPC = readRegister(instr.rs); break; // JR
                case 0x09: { // JALR
                    writeRegister(instr.rd, PC + 4);
                    nextPC = readRegister(instr.rs);
                    break;
                }
                case 0x10: 
                    writeRegister(instr.rd, getHI()); break; // MFHI
                case 0x11: 
                    setHI(readRegister(instr.rs)); break;   // MTHI
                case 0x12: 
                    writeRegister(instr.rd, getLO()); break; // MFLO
                case 0x13: 
                    setLO(readRegister(instr.rs)); break;   // MTLO
                case 0x18: { // MULT
                    int64_t rs_val = static_cast<int64_t>(static_cast<int32_t>(readRegister(instr.rs)));
                    int64_t rt_val = static_cast<int64_t>(static_cast<int32_t>(readRegister(instr.rt)));
                    int64_t prod = rs_val * rt_val;
                    setLO(static_cast<uint32_t>(prod & 0xFFFFFFFF));
                    setHI(static_cast<uint32_t>((prod >> 32) & 0xFFFFFFFF));
                    break;
                }
                case 0x19: { // MULTU
                    uint64_t rs_val = static_cast<uint64_t>(readRegister(instr.rs));
                    uint64_t rt_val = static_cast<uint64_t>(readRegister(instr.rt));
                    uint64_t prod = rs_val * rt_val;
                    setLO(static_cast<uint32_t>(prod & 0xFFFFFFFF));
                    setHI(static_cast<uint32_t>((prod >> 32) & 0xFFFFFFFF));
                    break;
                }
                case 0x1A: { // DIV
                    int32_t rs_val = static_cast<int32_t>(readRegister(instr.rs));
                    int32_t rt_val = static_cast<int32_t>(readRegister(instr.rt));
                    if (rt_val != 0) {
                        setLO(static_cast<uint32_t>(rs_val / rt_val));
                        setHI(static_cast<uint32_t>(rs_val % rt_val));
                    }
                    break;
                }
                case 0x1B: { // DIVU
                    uint32_t rs_val = readRegister(instr.rs);
                    uint32_t rt_val = readRegister(instr.rt);
                    if (rt_val != 0) {
                        setLO(rs_val / rt_val);
                        setHI(rs_val % rt_val);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }

        case 0x02: { // J
            uint32_t target = (PC & 0xF0000000) | (instr.target << 2);
            nextPC = target;
            break;
        }
        case 0x03: { // JAL
            writeRegister(31, PC + 4);
            uint32_t target = (PC & 0xF0000000) | (instr.target << 2);
            nextPC = target;
            break;
        }

        //  tipo I
        case 0x08: { // ADDI
            int32_t val = static_cast<int32_t>(readRegister(instr.rs));
            int32_t res = val + simm;
            if (((val ^ res) & (simm ^ res)) >= 0) writeRegister(instr.rt, res);
            break;
        }
        case 0x09: 
            writeRegister(instr.rt, readRegister(instr.rs) + simm);    
            break; // ADDIU
        case 0x0A: 
            writeRegister(instr.rt, (static_cast<int32_t>(readRegister(instr.rs)) < simm) ? 1 : 0);
            break; // SLTI
        case 0x0B: 
            writeRegister(instr.rt, (readRegister(instr.rs) < static_cast<uint32_t>(simm)) ? 1 : 0);
            break; // SLTIU
        case 0x0C: 
            writeRegister(instr.rt, readRegister(instr.rs) & uimm); 
            break; // ANDI
        case 0x0D: 
            writeRegister(instr.rt, readRegister(instr.rs) | uimm); 
            break; // ORI
        case 0x0E: 
            writeRegister(instr.rt, readRegister(instr.rs) ^ uimm); 
            break; // XORI
        case 0x0F: writeRegister(instr.rt, uimm << 16); break;                  // LUI

        // branche tipo I
        case 0x04: 
            if (readRegister(instr.rs) == readRegister(instr.rt)) 
                nextPC = (PC + 4) + (simm << 2); 
            break; // BEQ
        case 0x05: 
            if (readRegister(instr.rs) != readRegister(instr.rt)) 
                nextPC = (PC + 4) + (simm << 2); 
            break; // BNE
        case 0x06: 
            if (static_cast<int32_t>(readRegister(instr.rs)) <= 0) 
                nextPC = (PC + 4) + (simm << 2); 
            break; // BLEZ
        case 0x07: 
            if (static_cast<int32_t>(readRegister(instr.rs)) > 0)  nextPC = (PC + 4) + (simm << 2); break; // BGTZ

        case 0x20: { // LB
            int32_t addr = static_cast<int32_t>(readRegister(instr.rs)) + simm;
            uint8_t val = m_memory->readByte(static_cast<uint32_t>(addr));
            writeRegister(instr.rt, static_cast<int32_t>(static_cast<int8_t>(val)));
            break;
        }
        case 0x21: { // LH
            int32_t addr = static_cast<int32_t>(readRegister(instr.rs)) + simm;
            uint16_t val = m_memory->readHalf(static_cast<uint32_t>(addr));
            writeRegister(instr.rt, static_cast<int32_t>(static_cast<int16_t>(val)));
            break;
        }
        case 0x23: { // LW
            int32_t addr = static_cast<int32_t>(readRegister(instr.rs)) + simm;
            uint32_t val = m_memory->readWord(static_cast<uint32_t>(addr));
            writeRegister(instr.rt, val);
            break;
        }
        case 0x24: { // LBU
            int32_t addr = static_cast<int32_t>(readRegister(instr.rs)) + simm;
            uint8_t val = m_memory->readByte(static_cast<uint32_t>(addr));
            writeRegister(instr.rt, val);
            break;
        }
        case 0x25: { // LHU
            int32_t addr = static_cast<int32_t>(readRegister(instr.rs)) + simm;
            uint16_t val = m_memory->readHalf(static_cast<uint32_t>(addr));
            writeRegister(instr.rt, val);
            break;
        }
        case 0x28: { // SB
            int32_t addr = static_cast<int32_t>(readRegister(instr.rs)) + simm;
            uint8_t val = static_cast<uint8_t>(readRegister(instr.rt) & 0xFF);
            m_memory->writeByte(static_cast<uint32_t>(addr), val);
            break;
        }
        case 0x29: { // SH
            int32_t addr = static_cast<int32_t>(readRegister(instr.rs)) + simm;
            uint16_t val = static_cast<uint16_t>(readRegister(instr.rt) & 0xFFFF);
            m_memory->writeHalf(static_cast<uint32_t>(addr), val);
            break;
        }
        case 0x2B: { // SW
            int32_t addr = static_cast<int32_t>(readRegister(instr.rs)) + simm;
            uint32_t val = readRegister(instr.rt);
            m_memory->writeWord(static_cast<uint32_t>(addr), val);
            break;
        }

        default:
            break;
    }

    PC = nextPC;
}

void CPU::execute(uint32_t instr_word) {
    Instruction instr = decode(instr_word);
    execute(instr);
}

uint32_t CPU::fetch(const std::vector<uint32_t>& memory) {
    uint32_t index = PC / 4;
    if (index >= memory.size()) {
        stopped = true;
        throw std::out_of_range(std::format("PC (0x{:08X}) is out of instruction memory bounds ", PC, memory.size()));
    }
    return memory[index];
}
ALUResult CPU::executeTypeR(Instruction instr) {
    uint32_t opA = readRegister(instr.rs);
    uint32_t opB = readRegister(instr.rt);
    ALUControl control;

    switch (instr.funct) {
        case 0x20: control = ALUControl::ADD; break;
        case 0x21: control = ALUControl::ADDU; break;
        case 0x22: control = ALUControl::SUB; break;
        case 0x23: control = ALUControl::SUBU; break;
        case 0x24: control = ALUControl::AND; break;
        case 0x25: control = ALUControl::OR; break;
        case 0x26: control = ALUControl::XOR; break;
        case 0x27: control = ALUControl::NOR; break;
        case 0x2A: control = ALUControl::SLT; break;
        case 0x2B: control = ALUControl::SLTU; break;
        default:
            throw std::invalid_argument(std::format("Funct 0x{:02X} is not  valid ALU type-R operation", instr.funct));
    }

    return ALU::execute(opA, opB, control);
}

void CPU::writeBack(size_t reg, uint32_t value) {
    writeRegister(reg, value);
}

void CPU::step() {
    if (stopped) return;
    if (!m_memory) {
        stopped = true;
        return;
    }
    uint32_t instr_word = m_memory->fetchInstruction(PC);
    Instruction instr = decode(instr_word);
    execute(instr);
}
