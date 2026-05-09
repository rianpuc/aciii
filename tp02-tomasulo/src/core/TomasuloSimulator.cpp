#include "../../include/core/TomasuloSimulator.hpp"
#include "../../include/core/Instruction.hpp"
#include "../../include/utils/Parser.hpp"
#include "../../include/utils/Logger.hpp"
#include "../../include/utils/TomasuloException.hpp"
#include <iostream>
#include <iomanip>

TomasuloSimulator::TomasuloSimulator() {
    currentCycle = 0;
    isFinished = false;
    registerFile.resize(32, 0);
    memory.resize(1024, 0);
    int tagCounter = 1;
    for (int i = 0; i < 3; i++) addStations.push_back(ReservationStation(tagCounter++));
    for (int i = 0; i < 2; i++) mulStations.push_back(ReservationStation(tagCounter++));
    for (int i = 0; i < 2; i++) loadStoreStations.push_back(ReservationStation(tagCounter++));
}

void TomasuloSimulator::printState() {
    std::cout << "\n===========================================================\n";
    std::cout << "                   ESTADO NO CICLO " << currentCycle << "\n";
    std::cout << "===========================================================\n";

    std::cout << "--- ESTACOES DE RESERVA ---\n";
    std::cout << std::left 
              << std::setw(5) << "Tag" 
              << std::setw(6) << "Busy" 
              << std::setw(6) << "Op" 
              << std::setw(8) << "Vj" 
              << std::setw(8) << "Vk" 
              << std::setw(6) << "Qj" 
              << std::setw(6) << "Qk" 
              << std::setw(8) << "A" 
              << "\n";
    
    auto printRS = [](const ReservationStation& rs) {
        std::string opName = rs.busy ? getOpcodeName(rs.op) : "-";
        std::cout << std::left 
                  << std::setw(5) << rs.tag 
                  << std::setw(6) << (rs.busy ? "Sim" : "Nao")
                  << std::setw(6) << opName
                  << std::setw(8) << (rs.Qj == 0 && rs.busy ? std::to_string(rs.Vj) : "-") 
                  << std::setw(8) << (rs.Qk == 0 && rs.busy ? std::to_string(rs.Vk) : "-") 
                  << std::setw(6) << (rs.Qj != 0 && rs.busy ? std::to_string(rs.Qj) : "-")
                  << std::setw(6) << (rs.Qk != 0 && rs.busy ? std::to_string(rs.Qk) : "-")
                  << std::setw(8) << (rs.busy ? std::to_string(rs.A) : "-")
                  << "\n";
    };

    for (const auto& rs : addStations) printRS(rs);
    for (const auto& rs : mulStations) printRS(rs);
    for (const auto& rs : loadStoreStations) printRS(rs);

    std::cout << "\n--- BANCO DE REGISTRADORES E RAT ---\n";
    bool hasActiveRegisters = false;
    for (int i = 0; i < 32; i++) {
        int producer = rat.getProducer(i);
        if (producer != -1 || registerFile[i] != 0) {
            std::cout << "R" << i << " -> Valor Real: " << registerFile[i];
            if (producer != -1) {
                std::cout << " | Esperando Estacao de Reserva: " << producer;
            }
            std::cout << "\n";
            hasActiveRegisters = true;
        }
    }
    if (!hasActiveRegisters) std::cout << "Todos os registradores limpos (Valor 0).\n";
    std::cout << "===========================================================\n";
}

void TomasuloSimulator::loadInstructionsFromFile(const std::string& filename) {
    instructionQueue = Parser::parseFile(filename);
}

void TomasuloSimulator::run(const std::string& filename) {
    loadInstructionsFromFile(filename);
    Logger::log(currentCycle, Logger::INFO, "Iniciando Simulacao de Tomasulo...");
    printState();
    while (!isFinished) {
        currentCycle++;
        writeResult();
        execute();
        issue();
        printState();
        checkFinishCondition();
        if (currentCycle > 500) {
            Logger::log(currentCycle, Logger::ERROR, "Timeout! Possivel deadlock.");
            break;
        }
    }
    Logger::log(currentCycle, Logger::INFO, "Simulacao Concluida.");
}

void TomasuloSimulator::issue() {
    if (instructionQueue.empty()) return;
    Instruction inst = instructionQueue.front();
    ReservationStation* freeRS = nullptr;
    if (inst.op == ADD || inst.op == SUB) {
        for (auto& rs : addStations) {
            if (!rs.busy) { freeRS = &rs; break; }
        }
    } else if (inst.op == MUL || inst.op == DIV) {
        for (auto& rs : mulStations) {
            if (!rs.busy) { freeRS = &rs; break; }
        }
    } else if (inst.op == LW || inst.op == SW) {
        for (auto& rs : loadStoreStations) {
            if (!rs.busy) { freeRS = &rs; break; }
        }
    }
    if (freeRS == nullptr) {
        return;
    }
    instructionQueue.erase(instructionQueue.begin());
    freeRS->busy = true;
    freeRS->op = inst.op;
    if (inst.op == ADD || inst.op == SUB) freeRS->delayTimer = 1;
    if (inst.op == MUL) freeRS->delayTimer = 2;
    if (inst.op == DIV) freeRS->delayTimer = 2;
    if (inst.op == LW || inst.op == SW) freeRS->delayTimer = 2;
    if (inst.type == TYPE_R) {
        int producer1 = rat.getProducer(inst.srcRegister1);
        if (producer1 == -1) {
            freeRS->Vj = registerFile[inst.srcRegister1];
            freeRS->Qj = 0;
        } else {
            freeRS->Qj = producer1;
        }
        int producer2 = rat.getProducer(inst.srcRegister2);
        if (producer2 == -1) {
            freeRS->Vk = registerFile[inst.srcRegister2];
            freeRS->Qk = 0;
        } else {
            freeRS->Qk = producer2;
        }
        freeRS->result = inst.destRegister;
    }
    else if (inst.type == TYPE_I) {
        int producer1 = rat.getProducer(inst.srcRegister1);
        if (producer1 == -1) {
            freeRS->Vj = registerFile[inst.srcRegister1];
            freeRS->Qj = 0;
        } else {
            freeRS->Qj = producer1;
        }
        if (inst.op == SW) {
            int producer2 = rat.getProducer(inst.destRegister);
            if (producer2 == -1) { freeRS->Vk = registerFile[inst.destRegister]; freeRS->Qk = 0; }
            else { freeRS->Qk = producer2; }
        } else {
            freeRS->Qk = 0;
        }
        freeRS->A = inst.immediate;
        freeRS->result = inst.destRegister;
    }
    if (inst.op != SW) {
        rat.setProducer(inst.destRegister, freeRS->tag);
    }
    Logger::log(currentCycle, Logger::INFO, "Despacho feito: " + inst.rawText + " -> Estacao de Reserva: " + std::to_string(freeRS->tag));
}

void TomasuloSimulator::execute() {
    auto processRS = [&](ReservationStation& rs) {
        if (rs.busy && rs.Qj == 0 && rs.Qk == 0 && rs.delayTimer > 0) {
            rs.delayTimer--;
            if (rs.delayTimer == 0) {
                if (rs.op == ADD) rs.result = rs.Vj + rs.Vk;
                else if (rs.op == SUB) rs.result = rs.Vj - rs.Vk;
                else if (rs.op == MUL) rs.result = rs.Vj * rs.Vk;
                else if (rs.op == DIV) {
                    if (rs.Vk == 0) throw TomasuloException("Divisao por zero na Estacao de Reserva " + std::to_string(rs.tag));
                    rs.result = rs.Vj / rs.Vk;
                }
                else if (rs.op == LW) {
                    int addr = rs.Vj + rs.A;
                    rs.result = memory[addr];
                }
                else if (rs.op == SW) {
                    int addr = rs.Vj + rs.A;
                    memory[addr] = rs.Vk;
                    rs.clear();
                }
            }
        }
    };
    for (auto& rs : addStations) processRS(rs);
    for (auto& rs : mulStations) processRS(rs);
    for (auto& rs : loadStoreStations) processRS(rs);
}

void TomasuloSimulator::writeResult() {
    cdb.clear();
    ReservationStation* readyRS = nullptr;
    auto findReady = [&](std::vector<ReservationStation>& stations) {
        for (auto& rs : stations) {
            if (rs.busy && rs.delayTimer == 0) {
                readyRS = &rs;
                return;
            }
        }
    };
    findReady(addStations);
    if (!readyRS) findReady(mulStations);
    if (!readyRS) findReady(loadStoreStations);
    if (!readyRS) return;

    cdb.hasData = true;
    cdb.sourceReservationStation = readyRS->tag;
    cdb.resultValue = readyRS->result;

    Logger::log(currentCycle, Logger::DEBUG, "Dado salvo no CDB! Estacao de Reserva " + std::to_string(cdb.sourceReservationStation) +
                " finalizou com o valor: " + std::to_string(cdb.resultValue));
    readyRS->clear();
    for (int i = 0; i < 32; i++) {
        if (rat.getProducer(i) == cdb.sourceReservationStation) {
            registerFile[i] = cdb.resultValue;
            rat.clearDependency(i, cdb.sourceReservationStation);
        }
    }
    auto wakeUp = [&](std::vector<ReservationStation>& stations) {
        for (auto& rs : stations) {
            if (rs.busy) {
                if (rs.Qj == cdb.sourceReservationStation) { rs.Vj = cdb.resultValue; rs.Qj = 0; }
                if (rs.Qk == cdb.sourceReservationStation) { rs.Vk = cdb.resultValue; rs.Qk = 0; }
            }
        }
    };
    wakeUp(addStations);
    wakeUp(mulStations);
    wakeUp(loadStoreStations);
}

void TomasuloSimulator::checkFinishCondition() {
    if (!instructionQueue.empty()) return;
    for (const auto& rs : addStations) if (rs.busy) return;
    for (const auto& rs : mulStations) if (rs.busy) return;
    for (const auto& rs : loadStoreStations) if (rs.busy) return;
    isFinished = true;
}