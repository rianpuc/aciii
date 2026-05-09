#ifndef TOMASULO_SIMULATOR_HPP
#define TOMASULO_SIMULATOR_HPP

#include <vector>
#include <string>
#include "Instruction.hpp"
#include "../hardware/CommonDataBus.hpp"
#include "../hardware/ReservationStation.hpp"
#include "../hardware/RegisterAliasTable.hpp"

class TomasuloSimulator {
private:
    int currentCycle;
    bool isFinished;
    int issueWidth;
    std::vector<Instruction> instructionQueue;
    CommonDataBus cdb;
    RegisterAliasTable rat;
    std::vector<int> registerFile;
    std::vector<int> memory;
    std::vector<ReservationStation> addStations;
    std::vector<ReservationStation> mulStations;
    std::vector<ReservationStation> loadStoreStations;
    void loadInstructionsFromFile(const std::string& filename);
    void issue();
    void execute();
    void writeResult();
    void checkFinishCondition();
public:
    TomasuloSimulator();
    TomasuloSimulator(int numAdd, int numMul, int numLs, int issue);
    void printState();
    void run(const std::string& filename);
};

#endif