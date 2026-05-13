#include "../../include/core/TomasuloSimulator.hpp"
#include "../../include/core/Instruction.hpp"
#include "../../include/utils/Parser.hpp"
#include "../../include/utils/Logger.hpp"
#include "../../include/utils/TomasuloException.hpp"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#define MEMORY_SIZE 1024
#define ADD_SUB_CYCLES 1
#define MUL_DIV_CYCLES 2
#define LW_SW_CYCLES 2

TomasuloSimulator::TomasuloSimulator() {
    currentCycle = 0;
    isFinished = false;
    this->issueWidth = 2;
    int tagCounter = 1;
    memory.reserve(MEMORY_SIZE);
    registerFile.reserve(32);
    for (int i = 0; i < 32; i++) registerFile.push_back((rand() % 127) + 1);
    for (int i = 0; i < MEMORY_SIZE; i++) memory.push_back((rand() % 255) + 1);
    for (int i = 0; i < 2; i++) addStations.push_back(ReservationStation(tagCounter++));
    for (int i = 0; i < 1; i++) mulStations.push_back(ReservationStation(tagCounter++));
    for (int i = 0; i < 2; i++) loadStoreStations.push_back(ReservationStation(tagCounter++));
}

TomasuloSimulator::TomasuloSimulator(const int rsAdd, const int rsMul, const int rsLs, const int aluAdd, const int aluMul, const int aluLs, const int issue) {
    currentCycle = 0;
    isFinished = false;
    this->physicalAluAdd = aluAdd;
    this->physicalAluMul = aluMul;
    this->physicalAluLs = aluLs;
    this->issueWidth = issue;
    int tagCounter = 1;
    memory.reserve(MEMORY_SIZE);
    registerFile.reserve(32);
    for (int i = 0; i < 32; i++) registerFile.push_back((rand() % 127) + 1);
    for (int i = 0; i < MEMORY_SIZE; i++) memory.push_back((rand() % 255) + 1);
    for (int i = 0; i < rsAdd; i++) addStations.push_back(ReservationStation(tagCounter++));
    for (int i = 0; i < rsMul; i++) mulStations.push_back(ReservationStation(tagCounter++));
    for (int i = 0; i < rsLs; i++) loadStoreStations.push_back(ReservationStation(tagCounter++));
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
    int instructionsIssuedThisCycle = 0;
    while (!instructionQueue.empty() && instructionsIssuedThisCycle < this->issueWidth) {
        if (rob.size() >= robMaxSize) break;
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
        if (inst.op == ADD || inst.op == SUB) freeRS->delayTimer = ADD_SUB_CYCLES;
        if (inst.op == MUL || inst.op == DIV) freeRS->delayTimer = MUL_DIV_CYCLES;
        if (inst.op == LW || inst.op == SW) freeRS->delayTimer = LW_SW_CYCLES;
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
        instructionsIssuedThisCycle++;
        Logger::log(currentCycle, Logger::DEBUG, "Despacho feito: " + inst.rawText + " -> Estacao de Reserva: " + std::to_string(freeRS->tag));
    }
}

void TomasuloSimulator::execute() {
    int addAlusFree = this->physicalAluAdd;
    int mulAlusFree = this->physicalAluMul;
    int lsAlusFree  = this->physicalAluLs;
    auto processRS = [&](ReservationStation& rs, int& alusFree) {
        if (rs.busy && rs.Qj == 0 && rs.Qk == 0 && rs.delayTimer > 0) {
            if (alusFree > 0) {
                rs.delayTimer--;
                alusFree--;
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
                    }
                }
            }
        }
    };
    for (auto& rs : addStations) processRS(rs, addAlusFree);
    for (auto& rs : mulStations) processRS(rs, mulAlusFree);
    for (auto& rs : loadStoreStations) processRS(rs, lsAlusFree);
}

void TomasuloSimulator::writeResult() {
    std::vector<ReservationStation*> readyStations;
    auto findReady = [&](std::vector<ReservationStation>& stations) {
        for (auto& rs : stations) {
            if (rs.busy && rs.delayTimer == 0) {
                readyStations.push_back(&rs);
            }
        }
    };
    findReady(addStations);
    findReady(mulStations);
    findReady(loadStoreStations);

    if (readyStations.empty()) return;
    int busWidth = this->issueWidth;
    int broadcastsThisCycle = 0;
    for (ReservationStation* readyRS : readyStations) {
        if (broadcastsThisCycle >= busWidth) break;
        cdb.hasData = true;
        cdb.sourceReservationStation = readyRS->destROB;
        cdb.resultValue = readyRS->result;
        Logger::log(currentCycle, Logger::DEBUG, "Dado salvo no CDB! ROB Tag " + std::to_string(cdb.sourceReservationStation) +
            " finalizou com o valor: " + std::to_string(cdb.resultValue));
        for (auto& entry : rob) {
            if (entry.tag == cdb.sourceReservationStation) {
                entry.value = cdb.resultValue;
                entry.ready = true;
                entry.state = WRITE_RESULT;
                if (entry.inst.op == SW) entry.destination = readyRS->Vj + readyRS->A;
                break;
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
        readyRS->clear();
        broadcastsThisCycle++;
    }
}

void TomasuloSimulator::commit() {
    int commitsThisCycle = 0;
    while (!rob.empty() && commitsThisCycle < this->issueWidth) {
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
        commitsThisCycle++;
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