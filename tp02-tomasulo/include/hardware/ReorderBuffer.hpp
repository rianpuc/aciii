#ifndef REORDER_BUFFER_HPP
#define REORDER_BUFFER_HPP
#include "../core/Instruction.hpp"
enum RobState { ISSUE, EXECUTE, WRITE_RESULT, COMMITTED };

inline std::string getStateName(RobState state) {
    switch (state) {
        case ISSUE: return "Issue";
        case EXECUTE: return "Execute";
        case WRITE_RESULT: return "Write result";
        case COMMITTED: return "Committed";
        default: return "Unknown";
    }
}

struct ReorderBufferEntry {
    int tag;
    Instruction inst;
    RobState state;
    int destination;
    int value;
    bool ready;
    ReorderBufferEntry(int t, Instruction i) 
        : tag(t), inst(i), state(ISSUE), destination(i.destRegister), value(0), ready(false) {}
    std::string getStateName() {
        switch (this->state) {
            case ISSUE: return "Issue";
            case EXECUTE: return "Execute";
            case WRITE_RESULT: return "Write result";
            case COMMITTED: return "Committed";
            default: return "Unknown";
        }
    }
};

#endif