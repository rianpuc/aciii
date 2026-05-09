#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include <sstream>

enum Opcode { ADD, SUB, MUL, DIV, LW, SW };

inline std::string getOpcodeName(Opcode op) {
    switch (op) {
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case LW:  return "LW";
        case SW:  return "SW";
        default:  return "UNKNOWN";
    }
}

enum InstType { TYPE_R, TYPE_I };

struct Instruction {
    int id;
    Opcode op;
    InstType type;
    int destRegister;
    int srcRegister1;
    int srcRegister2;
    int immediate;
    std::string rawText;  // TO SALVANDO O TEXTO ORIGINAL SO PRA DEBUGAR
    std::string toString() const {
        std::stringstream ss;
        ss << "[I" << id << "] " << getOpcodeName(op) <<  " -> ";
        if (type == TYPE_R) {
            ss << "Dest: R" << destRegister
               << " | Src1: R" << srcRegister1
               << " | Src2: R" << srcRegister2;
        } else if (type == TYPE_I) {
            ss << "Dest: R" << destRegister
               << " | Base: R" << srcRegister1
               << " | Offset: " << immediate;
        }
        ss << " (Raw: \"" << rawText << "\")";
        return ss.str();
    }
};

#endif