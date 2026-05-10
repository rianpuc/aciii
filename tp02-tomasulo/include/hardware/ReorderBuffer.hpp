#ifndef REORDER_BUFFER_HPP
#define REORDER_BUFFER_HPP
#include "../core/Instruction.hpp"
enum RobState { ISSUE, EXECUTE, WRITE_RESULT, COMMITTED };

struct ReorderBufferEntry {
    int tag;
    Instruction inst;
    RobState state;
    int destination;
    int value;
    bool ready;
    ReorderBufferEntry(int t, Instruction i) 
        : tag(t), inst(i), state(ISSUE), destination(i.destRegister), value(0), ready(false) {}
};

#endif