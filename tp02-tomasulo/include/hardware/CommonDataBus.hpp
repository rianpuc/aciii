#ifndef COMMON_DATA_BUS_HPP
#define COMMON_DATA_BUS_HPP

struct CommonDataBus {
    bool hasData;
    int sourceReservationStation;
    int resultValue;

    CommonDataBus() : hasData(false), sourceReservationStation(-1), resultValue(0) {}

    void clear() {
        hasData = false;
        sourceReservationStation = -1;
        resultValue = 0;
    }
};

#endif