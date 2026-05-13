#include "../../include/hardware/FunctionalUnit.hpp"
#include "../../include/core/TomasuloSimulator.hpp"
#include "../../include/core/Instruction.hpp"
#include "../../include/utils/Parser.hpp"
#include "../../include/utils/Logger.hpp"
#include "../../include/utils/TomasuloException.hpp"
#include <iostream>
#include <iomanip>
#define MEMORY_SIZE 1024
#define ADD_SUB_CYCLES 1
#define MUL_DIV_CYCLES 2
#define LW_SW_CYCLES 2

TomasuloSimulator::TomasuloSimulator(const int rsAdd, const int rsMul, const int rsLs, const int aluAdd, const int aluMul, const int aluLs, const int issue) {
    currentCycle = 0;
    isFinished = false;
    this->issueWidth = issue;
    memory.reserve(MEMORY_SIZE);
	fuAluAdd.reserve(aluAdd);
	fuAluMul.reserve(aluMul);
	fuAluLS.reserve(aluLs);
    registerFile.reserve(32);
    for (int i = 0; i < 32; i++) registerFile.push_back((rand() % 127) + 1);
	for (int i = 0; i < aluAdd; i++) fuAluAdd.push_back(FunctionalUnit("ADD" + std::to_string(i+1), ADD_SUB_CYCLES));
	for (int i = 0; i < aluMul; i++) fuAluMul.push_back(FunctionalUnit("MUL" + std::to_string(i+1), MUL_DIV_CYCLES));
	for (int i = 0; i < aluLs; i++) fuAluLS.push_back(FunctionalUnit("LS" + std::to_string(i+1), LW_SW_CYCLES));
    for (int i = 0; i < MEMORY_SIZE; i++) memory.push_back((rand() % 255) + 1);
    for (int i = 0; i < rsAdd; i++) addStations.push_back(ReservationStation("rsAdd" + std::to_string(i+1)));
    for (int i = 0; i < rsMul; i++) mulStations.push_back(ReservationStation("rsMul" + std::to_string(i+1)));
    for (int i = 0; i < rsLs; i++) loadStoreStations.push_back(ReservationStation("rsLS" + std::to_string(i+1)));
}

void TomasuloSimulator::printState() {
    Logger::log(Logger::INFO, "===========================================================================");
    Logger::log(Logger::INFO, "                            ESTADO NO CICLO " + std::to_string(currentCycle));
    Logger::log(Logger::INFO, "===========================================================================");
    Logger::log(Logger::INFO, "------------------------------ REORDER BUFFER ------------------------------");
	std::stringstream robHeaders, robLine;
	robHeaders << std::left
               << std::setw(10) << "Entry"
               << std::setw(10) << "Ready"
    		   << std::setw(20) << "Instruction"
			   << std::setw(20) << "State"
    		   << std::setw(10) << "Dest"
			   << std::setw(10) << "Value";
	Logger::log(Logger::INFO, robHeaders.str());
	if (!rob.empty()){
		for(auto& entry : rob){
			std::stringstream valueLine;
			if (entry.inst.type == TYPE_I){
				valueLine << ("Mem[" + std::to_string(entry.inst.immediate) + " + R" + std::to_string(entry.inst.srcRegister1) + "]");
			} else {
				int valueRegister1 = rat.getProducer(entry.inst.srcRegister1);
				int valueRegister2 = rat.getProducer(entry.inst.srcRegister2);
				std::string srcReg1 = valueRegister1 == -1 ? "R" + std::to_string(entry.inst.srcRegister1) : "#" + std::to_string(valueRegister1);
				std::string srcReg2 = valueRegister2 == -1 ? "R" + std::to_string(entry.inst.srcRegister2) : "#" + std::to_string(valueRegister2);
				valueLine << (srcReg1 + " " + entry.inst.getOperator() + " " + srcReg2);
			}
			robLine << std::left
			    	<< std::setw(10) << std::to_string(entry.tag)
					<< std::setw(10) << (entry.ready ? "Yes" : "No")
			   		<< std::setw(20) << entry.inst.rawText
					<< std::setw(20) << entry.getStateName()
					<< std::setw(10) << ("R" + std::to_string(entry.destination))
					<< std::setw(10) << valueLine.str();
			Logger::log(Logger::INFO, robLine.str());
	    	robLine.str("");
			valueLine.str("");
        	robLine.clear();
			valueLine.clear();
		}
	} else {
		Logger::log(Logger::INFO, "");
	}
    Logger::log(Logger::INFO, "--------------------------- ESTACOES DE RESERVA ---------------------------");
    std::stringstream rsHeaders, rsLine;
    rsHeaders << std::left
              << std::setw(8) << "Name"
              << std::setw(8) << "Busy"
              << std::setw(8) << "Op"
              << std::setw(8) << "Vj"
              << std::setw(8) << "Vk"
              << std::setw(8) << "Qj"
              << std::setw(8) << "Qk"
              << std::setw(8) << "Dest"
              << std::setw(8) << "A";
    Logger::log(Logger::INFO, rsHeaders.str());
    auto printRS = [&rsLine](const ReservationStation& rs) {
        rsLine.str("");
        rsLine.clear();
        std::string opName = rs.busy ? getOpcodeName(rs.op) : "-";
        rsLine << std::left
                  << std::setw(8) << rs.tag
                  << std::setw(8) << (rs.busy ? "Sim" : "Nao")
                  << std::setw(8) << opName
                  << std::setw(8) << (rs.Qj == 0 && rs.busy ? std::to_string(rs.Vj) : "-")
                  << std::setw(8) << (rs.Qk == 0 && rs.busy ? std::to_string(rs.Vk) : "-")
                  << std::setw(8) << (rs.Qj != 0 && rs.busy ? std::to_string(rs.Qj) : "-")
                  << std::setw(8) << (rs.Qk != 0 && rs.busy ? std::to_string(rs.Qk) : "-")
                  << std::setw(8) << (rs.destROB != 0 && rs.busy ? std::to_string(rs.destROB) : "-")
                  << std::setw(8) << (rs.busy ? std::to_string(rs.A) : "-");
        Logger::log(Logger::INFO, rsLine.str());
    };

    for (const auto& rs : addStations) printRS(rs);
    for (const auto& rs : mulStations) printRS(rs);
    for (const auto& rs : loadStoreStations) printRS(rs);

    const int CHUNK_SIZE = 8;
    Logger::log(Logger::INFO, "------------------------- STATUS DOS REGISTRADORES -------------------------");
    for (int start = 0; start < 32; start += CHUNK_SIZE) {
        std::stringstream headerLine, producerLine, busyLine;
        headerLine << std::left << std::setw(12) << "Field";
        producerLine << std::left << std::setw(12) << "Reorder #";
        busyLine << std::left << std::setw(12) << "Busy";
        for (int i = start; i < start + CHUNK_SIZE && i < 32; i++) {
            headerLine << std::left << std::setw(8) << ("R" + std::to_string(i));
            int producer = rat.getProducer(i);
            if (producer != -1) {
                producerLine << std::left << std::setw(8) << producer;
                busyLine << std::left << std::setw(8) << "Yes";
            } else {
                producerLine << std::left << std::setw(8) << "-";
                busyLine << std::left << std::setw(8) << "No";
            }
        }
        Logger::log(Logger::INFO, headerLine.str());
        Logger::log(Logger::INFO, producerLine.str());
        Logger::log(Logger::INFO, busyLine.str());
        Logger::log(Logger::INFO, "----------------------------------------------------------------------------");
    }
}

void TomasuloSimulator::loadInstructionsFromFile(const std::string& filename) {
    instructionQueue = Parser::parseFile(filename);
}

void TomasuloSimulator::run(const std::string& filename) {
    loadInstructionsFromFile(filename);
    Logger::log(Logger::INFO, "Iniciando Simulacao de Tomasulo...");
    while (!isFinished) {
        currentCycle++;
        commit();
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
    Logger::log(Logger::INFO, "Simulacao Concluida no Ciclo " + std::to_string(currentCycle));
}

void TomasuloSimulator::issue() {
    while (!instructionQueue.empty()) {
        Instruction inst = instructionQueue.front();
        ReservationStation* freeRS = nullptr;
        if (inst.op == ADD || inst.op == SUB) {
            for (auto& rs : addStations) if (!rs.busy) { freeRS = &rs; break; }
        } else if (inst.op == MUL || inst.op == DIV) {
            for (auto& rs : mulStations) if (!rs.busy) { freeRS = &rs; break; }
        } else if (inst.op == LW || inst.op == SW) {
            for (auto& rs : loadStoreStations) if (!rs.busy) { freeRS = &rs; break; }
        }
        if (freeRS == nullptr) break;
        instructionQueue.erase(instructionQueue.begin());
        freeRS->busy = true;
        freeRS->op = inst.op;
        int myRobTag = robTagCounter++;
        rob.push_back(ReorderBufferEntry(myRobTag, inst));
        freeRS->destROB = myRobTag;
        auto readOperand = [&](int regNumber, int& V, int& Q) {
            int producerRobTag = rat.getProducer(regNumber);
            if (producerRobTag == -1) {
                V = registerFile.at(regNumber);
                Q = 0;
            } else {
                bool foundReadyInRob = false;
                for (auto& entry : rob) {
                    if (entry.tag == producerRobTag && entry.ready) {
                        V = entry.value;
                        Q = 0;
                        foundReadyInRob = true;
                        break;
                    }
                }
                if (!foundReadyInRob) Q = producerRobTag;
            }
        };
        if (inst.type == TYPE_R) {
            readOperand(inst.srcRegister1, freeRS->Vj, freeRS->Qj);
            readOperand(inst.srcRegister2, freeRS->Vk, freeRS->Qk);
        }
        else if (inst.type == TYPE_I) {
            readOperand(inst.srcRegister1, freeRS->Vj, freeRS->Qj);
            if (inst.op == SW) readOperand(inst.destRegister, freeRS->Vk, freeRS->Qk);
            else freeRS->Qk = 0;
            freeRS->A = inst.immediate;
        }
        if (inst.op != SW) {
            rat.setProducer(inst.destRegister, myRobTag);
        }
        Logger::log(currentCycle, Logger::DEBUG, "Nova instrucao: " + inst.rawText + " -> Estacao de Reserva: " + freeRS->tag);
    }
}

void TomasuloSimulator::execute() {
	for (auto& fu : fuAluAdd) fu.tick();
    for (auto& fu : fuAluMul) fu.tick();
    for (auto& fu : fuAluLS)  fu.tick();
	auto getFreeFU = [](std::vector<FunctionalUnit>& alus) -> FunctionalUnit* {
        for (auto& fu : alus) {
            if (!fu.busy) return &fu;
        }
        return nullptr;
    };
	auto tryDispatch = [&](ReservationStation& rs, std::vector<FunctionalUnit>& alus) {
        if (rs.busy && rs.Qj == 0 && rs.Qk == 0) {
            FunctionalUnit* freeFU = getFreeFU(alus);
            if (freeFU != nullptr) {
                int v2 = (rs.op == LW || rs.op == SW) ? rs.A : rs.Vk;
                freeFU->dispatch(rs.op, rs.Vj, v2, rs.destROB);
				Logger::log(currentCycle, Logger::DEBUG, "Instrucao despachada: " + rs.tag + " -> Functional Unit: " + freeFU->tag);
                rs.clear();
            }
        }
    };
	for (auto& rs : addStations) tryDispatch(rs, fuAluAdd);
    for (auto& rs : mulStations) tryDispatch(rs, fuAluMul);
    for (auto& rs : loadStoreStations) tryDispatch(rs, fuAluLS);
}

void TomasuloSimulator::writeResult() {
    std::vector<FunctionalUnit*> readyFUS;
    auto findReady = [&](std::vector<FunctionalUnit>& fus) {
        for (auto& fu : fus) {
            if (fu.resultReady && fu.cyclesLeft == 0) {
                readyFUS.push_back(&fu);
            }
        }
    };
    findReady(fuAluAdd);
    findReady(fuAluMul);
    findReady(fuAluLS);
    if (readyFUS.empty()) return;
    for (FunctionalUnit* readyFU : readyFUS) {
		int finalBroadcastValue = readyFU->result;
        bool broadcastToCDB = true;
		if (readyFU->op == LW) {
            int address = readyFU->result;
            finalBroadcastValue = memory[address];
        }
        else if (readyFU->op == SW) {
            broadcastToCDB = false;
        }
        for (auto& entry : rob) {
            if (entry.tag == readyFU->destTag) {
                entry.ready = true;
                entry.state = WRITE_RESULT;
                if (readyFU->op == SW) entry.destination = readyFU->result;
				else entry.value = finalBroadcastValue;
                break;
            }
        }
		if (broadcastToCDB) {
            cdb.hasData = true;
            cdb.sourceReservationStation = readyFU->destTag;
            cdb.resultValue = finalBroadcastValue;
            Logger::log(currentCycle, Logger::DEBUG, "Dado salvo no CDB! " +
                readyFU->tag +
                " finalizou com o valor: " + std::to_string(cdb.resultValue));
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
        } else {
            Logger::log(currentCycle, Logger::DEBUG, "SW Tag " + std::to_string(readyFU->destTag) +
                " calculou o endereço e aguarda no ROB.");
        }
        readyFU->clear();
    }
}

void TomasuloSimulator::commit() {
    while (!rob.empty()) {
        ReorderBufferEntry& head = rob.front();
        if (!head.ready) break;

        if (head.inst.op != SW) {
            registerFile.at(head.destination) = head.value;
            if (rat.getProducer(head.destination) == head.tag) {
                rat.clearDependency(head.destination, head.tag);
            }
            Logger::log(currentCycle, Logger::DEBUG, "COMMIT! " + head.inst.rawText +
                        " foi oficializada e gravou o valor " + std::to_string(head.value) +
                        " no Reg R" + std::to_string(head.destination));
        }
        else {
            memory.at(head.destination) = head.value;
            Logger::log(currentCycle, Logger::DEBUG, "COMMIT! " + head.inst.rawText +
                        " foi oficializada e gravou na RAM[" + std::to_string(head.destination) + "]");
        }
        head.state = COMMITTED;
        rob.pop_front();
    }
}

void TomasuloSimulator::checkFinishCondition() {
    if (!instructionQueue.empty()) return;
    if (!rob.empty()) return;
    for (const auto& rs : addStations) if (rs.busy) return;
    for (const auto& rs : mulStations) if (rs.busy) return;
    for (const auto& rs : loadStoreStations) if (rs.busy) return;
    isFinished = true;
}