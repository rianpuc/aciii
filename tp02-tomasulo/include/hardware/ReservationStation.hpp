#ifndef RESERVATION_STATION_HPP
#define RESERVATION_STATION_HPP

#include "../core/Instruction.hpp"

struct ReservationStation {
    int tag;
    bool busy;
    Opcode op;
    int Vj;
    int Vk;
    int Qj;
    int Qk;
    int A;
    int result;
    int delayTimer;
    ReservationStation(int t) : tag(t) {
        clear();
    }
    void clear() {
        busy = false;
        op = ADD;
        Vj = 0; Vk = 0;
        Qj = 0; Qk = 0;
        A = 0;
        result = 0;
        delayTimer = -1;
    }
    bool isReady() const {
        return busy && (Qj == 0) && (Qk == 0);
    }
};

#endif