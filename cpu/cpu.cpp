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
