#ifndef RESERVATION_STATION_HPP
#define RESERVATION_STATION_HPP

#include "../core/Instruction.hpp"

struct ReservationStation {
    std::string tag;
    bool busy;
    Opcode op;
    int Vj;
    int Vk;
    int Qj;
    int Qk;
    int destROB;
    int A;
    int result;
    int delayTimer;
    std::string instruction;
    ReservationStation(std::string t) : tag(t) {
        clear();
    }
    void clear() {
        busy = false;
        op = ADD;
        Vj = 0; Vk = 0;
        Qj = 0; Qk = 0;
        A = -1;
        result = 0;
        destROB = 0;
        delayTimer = -1;
        instruction = "";
    }
    bool isReady() const {
        return busy && (Qj == 0) && (Qk == 0);
    }
};

#endif