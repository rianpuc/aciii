#ifndef TOMASULO_SIMULATOR_HPP
#define TOMASULO_SIMULATOR_HPP

#include <deque>
#include <vector>
#include <string>
#include "Instruction.hpp"
#include "../hardware/ReorderBuffer.hpp"
#include "../hardware/CommonDataBus.hpp"
#include "../hardware/ReservationStation.hpp"
#include "../hardware/FunctionalUnit.hpp"
#include "../hardware/RegisterAliasTable.hpp"

class TomasuloSimulator {
private:
    int currentCycle;
    bool isFinished;
    int cdbWidth;
    int latencyAdd;
    int latencyMul;
    int latencyLS;
    std::vector<FunctionalUnit> fuAluAdd;
    std::vector<FunctionalUnit> fuAluMul;
    std::vector<FunctionalUnit> fuAluLS;
    std::deque<Instruction> instructionQueue;
    CommonDataBus cdb;
    RegisterAliasTable rat;
    std::vector<int> registerFile;
    std::vector<int> memory;
    std::vector<ReservationStation> addStations;
    std::vector<ReservationStation> mulStations;
    std::vector<ReservationStation> loadStoreStations;
    std::deque<ReorderBufferEntry> rob;
    int robTagCounter = 1;
    void loadInstructionsFromFile(const std::string& filename);
    void issue();
    void execute();
    void writeResult();
    void commit();
    void checkFinishCondition();
public:
    TomasuloSimulator(int rsAdd, int rsMul, int rsLs, int aluAdd, int aluMul, int aluLs, int cdbWidth, int latAdd, int latMul, int latLs);
    int getRegistersSize();
    int getMemorySize();
    void printState();
    void run(const std::string& filename);
    void setRegisterValue(int regIndex, int value);
    void setMemoryValue(int memAddress, int value);
};

#endif