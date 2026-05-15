#ifndef FUNCTIONAL_UNIT_HPP
#define FUNCTIONAL_UNIT_HPP
#include "../core/Instruction.hpp"
#include "../utils/TomasuloException.hpp"

class FunctionalUnit {
public:
    std::string tag;
    bool busy;
    bool resultReady;
    Opcode op;
    int cycles;
    int cyclesLeft;
    int val1, val2;
    int destTag;
    int result;
    int A;
    std::string rawInstruction;
    FunctionalUnit(std::string tag, int cycles) {
        this->tag = tag;
        this->cycles = cycles;
        this->cyclesLeft = cycles;
        clear();
    }
    void dispatch(Opcode operation, int v1, int v2, int tag, int imm, std::string instruction) {
        op = operation;
        val1 = v1;
        val2 = v2;
        destTag = tag;
        cyclesLeft = cycles;
        A = imm;
        rawInstruction = instruction;
        busy = true;
        resultReady = false;
    }
    void tick() {
        if (!busy || resultReady) return;
        cyclesLeft--;
        if (cyclesLeft == 0) {
            switch (op) {
                case ADD: result = val1 + val2; break;
                case SUB: result = val1 - val2; break;
                case MUL: result = val1 * val2; break;
                case DIV:
                    if (val2 == 0) throw TomasuloException("Divisao por zero na FU " + tag);
                    result = val1 / val2;
                    break;
                case LW: result = val1 + A; break;
                case SW: result = val1 + A; break;
            }
            resultReady = true;
        }
    }
    void clear() {
        busy = false;
        resultReady = false;
        cyclesLeft = cycles;
        rawInstruction = "";
        val1 = val2 = destTag = result = A = 0;
    }
};

#endif