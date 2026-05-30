#include "../../include/hardware/FunctionalUnit.hpp"
#include "../../include/core/TomasuloSimulator.hpp"
#include "../../include/core/Instruction.hpp"
#include "../../include/utils/Parser.hpp"
#include "../../include/utils/Logger.hpp"
#include "../../include/utils/TomasuloException.hpp"
#include <iostream>
#include <iomanip>
#define MEMORY_SIZE 1024
#pragma GCC diagnostic ignored "-Wsign-compare"

TomasuloSimulator::TomasuloSimulator(const int rsAdd, const int rsMul, const int rsLs, const int aluAdd, const int aluMul, const int aluLs, const int cdbWidth, const int latAdd, const int latMul, const int latLs) {
    currentCycle = 0;
    isFinished = false;
    this->cdbWidth = cdbWidth;
    memory.reserve(MEMORY_SIZE);
	fuAluAdd.reserve(aluAdd);
	fuAluMul.reserve(aluMul);
	fuAluLS.reserve(aluLs);
    registerFile.reserve(32);
    for (int i = 0; i < 32; i++) registerFile.push_back((rand() % 15) + 1);
	for (int i = 0; i < aluAdd; i++) fuAluAdd.push_back(FunctionalUnit("ADD" + std::to_string(i+1), latAdd));
	for (int i = 0; i < aluMul; i++) fuAluMul.push_back(FunctionalUnit("MUL" + std::to_string(i+1), latMul));
	for (int i = 0; i < aluLs; i++) fuAluLS.push_back(FunctionalUnit("LS" + std::to_string(i+1), latLs));
    for (int i = 0; i < MEMORY_SIZE; i++) memory.push_back((rand() % 7) + 1);
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
			   << std::setw(18) << "State"
    		   << std::setw(15) << "Destination"
			   << std::setw(10) << "Value";
	Logger::log(Logger::INFO, robHeaders.str());
	if (!rob.empty()){
		for(auto& entry : rob){
			std::stringstream valueLine;
			switch (entry.inst.op) {
			    case LW: {
			        int valueRegisterBase = rat.getProducer(entry.inst.srcRegister1);
			        std::string baseRegStr;
			        if (valueRegisterBase != -1 && entry.tag > valueRegisterBase) {
			            baseRegStr = "#" + std::to_string(valueRegisterBase);
			        } else {
			            baseRegStr = "R" + std::to_string(entry.inst.srcRegister1);
			        }
			        valueLine << ("Mem[" + std::to_string(entry.inst.immediate) + " + " + baseRegStr + "]");
			        break;
			    }
			    case SW: {
			        int valueRegisterSW = rat.getProducer(entry.inst.destRegister);
			        if (valueRegisterSW != -1 && entry.tag > valueRegisterSW) {
			            valueLine << ("#" + std::to_string(valueRegisterSW));
			        } else {
			            valueLine << ("R" + std::to_string(entry.inst.destRegister));
			        }
			        break;
			    }
			    default: {
			        int valueRegister1 = rat.getProducer(entry.inst.srcRegister1);
			        int valueRegister2 = rat.getProducer(entry.inst.srcRegister2);
			        std::string srcReg1 = valueRegister1 != -1 && entry.tag > valueRegister1 ? "#" + std::to_string(valueRegister1) : "R" + std::to_string(entry.inst.srcRegister1);
			        std::string srcReg2 = valueRegister2 != -1 && entry.tag > valueRegister2 ? "#" + std::to_string(valueRegister2) : "R" + std::to_string(entry.inst.srcRegister2);
			        valueLine << (srcReg1 + " " + entry.inst.getOperator() + " " + srcReg2);
			        break;
			    }
			}
		    std::string destination = entry.inst.op == SW ? "Mem[" + std::to_string(entry.inst.immediate) + " + R" + std::to_string(entry.inst.srcRegister1) + "]" : "R" + std::to_string(entry.destination);
			robLine << std::left
			    	<< std::setw(10) << std::to_string(entry.tag)
					<< std::setw(10) << (entry.ready ? "Yes" : "No")
			   		<< std::setw(20) << entry.inst.rawText
					<< std::setw(18) << entry.getStateName()
					<< std::setw(15) << destination
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
        std::string vkStr = "-";
        std::string qkStr = "-";
        if (rs.busy) {
            if (rs.op == LW) {
                vkStr = "-";
                qkStr = "-";
            } else {
                vkStr = (rs.Qk == 0) ? std::to_string(rs.Vk) : "-";
                qkStr = (rs.Qk != 0) ? "#" + std::to_string(rs.Qk) : "-";
            }
        }
        std::string opName = rs.busy ? getOpcodeName(rs.op) : "-";
        rsLine << std::left
                  << std::setw(8) << rs.tag
                  << std::setw(8) << (rs.busy ? "Yes" : "No")
                  << std::setw(8) << opName
                  << std::setw(8) << (rs.Qj == 0 && rs.busy ? std::to_string(rs.Vj) : "-")
                  << std::setw(8) << vkStr
                  << std::setw(8) << (rs.Qj != 0 && rs.busy ? "#" + std::to_string(rs.Qj) : "-")
                  << std::setw(8) << qkStr
                  << std::setw(8) << (rs.destROB != 0 && rs.busy ? "#" + std::to_string(rs.destROB) : "-")
                  << std::setw(8) << (rs.A != -1 ? std::to_string(rs.A) : "-");
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
        freeRS->instruction = inst.rawText;
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
            if (inst.op == SW) {
                readOperand(inst.destRegister, freeRS->Vk, freeRS->Qk);
            }
            else freeRS->Qk = 0;
            freeRS->A = inst.immediate;
        }
        if (inst.op != SW) {
            rat.setProducer(inst.destRegister, myRobTag);
        }
        if (inst.op == SW && freeRS->Qj == 0) {
            rob.back().effectiveAddress = freeRS->Vj + inst.immediate;
            rob.back().addressReady = true;
        }
        Logger::log(currentCycle, Logger::DEBUG, "Nova instrucao: " + inst.rawText + " enviada para ReservationStation: " + freeRS->tag);
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
            if (rs.op == LW) {
                int lwAddr = rs.Vj + rs.A;
                int lwPhysicalIndex = -1;
                for (int i = 0; i < rob.size(); i++) {
                    if (rob[i].tag == rs.destROB) {
                        lwPhysicalIndex = i;
                        break;
                    }
                }
                if (lwPhysicalIndex != -1) {
                    for (int i = lwPhysicalIndex - 1; i >= 0; i--) {
                        auto& entry = rob[i];
                        if (entry.state == COMMITTED) break;

                        if (entry.inst.op == SW) {
                            if (!entry.addressReady) return;
                            if (entry.effectiveAddress == lwAddr) {
                                if (entry.ready) {
                                    rs.Vj = entry.value;
                                    rs.bypassMemory = true;
                                    Logger::log(currentCycle, Logger::DEBUG,"Bypass! Roubando valor " + std::to_string(entry.value) + " do " + entry.inst.rawText + " para: " + rs.instruction);
                                    break;
                                } else {
                                    return;
                                }
                            }
                        }
                    }
                }
            }
            FunctionalUnit* freeFU = getFreeFU(alus);
            if (freeFU != nullptr) {
                freeFU->dispatch(rs.op, rs.Vj, rs.Vk, rs.destROB, rs.A, rs.instruction, rs.bypassMemory);
                for (auto& entry : rob) {
                    if (entry.tag == freeFU->destTag) {
                        entry.state = EXECUTE;
                        break;
                    }
                }
				Logger::log(currentCycle, Logger::DEBUG, "Instrucao " + rs.instruction + " da ReservationStation " + rs.tag + " despachada para a Functional Unit: " + freeFU->tag);
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
    int writesThisCycle = 0;
    for (FunctionalUnit* readyFU : readyFUS) {
        if (writesThisCycle >= cdbWidth) {
            Logger::log(currentCycle, Logger::DEBUG, "STALL ON CDB: " + readyFU->rawInstruction + " finalizou, mas o barramento esta lotado! Aguardando...");
            break;
        }
		int finalBroadcastValue = readyFU->result;
        bool broadcastToCDB = true;
		if (readyFU->op == LW) {
            int address = readyFU->result;
		    if (readyFU->bypassMem) {
		        finalBroadcastValue = address;
		    } else {
		        int rawAddress = readyFU->val1 + readyFU->A;
		        int safeAddress = std::abs(rawAddress) % MEMORY_SIZE;
		        if (rawAddress < 0 || rawAddress >= MEMORY_SIZE) {
		            Logger::log(Logger::WARNING, "Excecao de Hardware! Endereco " + std::to_string(rawAddress) +
                                " fora dos limites. MMU redirecionando para endereco seguro " + std::to_string(safeAddress));
		            finalBroadcastValue = memory[safeAddress];
		        } else {
		            finalBroadcastValue = memory[address];
		        }
		    }
        }
        else if (readyFU->op == SW) {
            broadcastToCDB = false;
        }
        for (auto& entry : rob) {
            if (entry.tag == readyFU->destTag) {
                entry.ready = true;
                entry.state = WRITE_RESULT;
                if (readyFU->op == SW) {
                    entry.destination = readyFU->result;
                    entry.value = readyFU->val2;
                    entry.effectiveAddress = readyFU->result;
                    entry.addressReady = true;
                }
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
                " finalizou a operacao: " + readyFU->rawInstruction + " com o valor: " + std::to_string(cdb.resultValue));
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
            Logger::log(currentCycle, Logger::DEBUG, readyFU->tag +
                " finalizou a operacao: " + readyFU->rawInstruction + " e agora aguarda no RoB.");
        }
        readyFU->clear();
        writesThisCycle++;
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
            int rawAddress = head.effectiveAddress;
            int safeAddress = std::abs(rawAddress) % MEMORY_SIZE;
            if (rawAddress < 0 || rawAddress >= MEMORY_SIZE) {
                Logger::log(Logger::WARNING, "Excecao de Hardware! Endereco " + std::to_string(rawAddress) +
                            " fora dos limites. MMU redirecionando para endereco seguro " + std::to_string(safeAddress));
                memory.at(safeAddress) = head.value;
            } else {
                memory.at(rawAddress) = head.value;
            }
            rawAddress = safeAddress;
            Logger::log(currentCycle, Logger::DEBUG, "COMMIT! " + head.inst.rawText +
            " foi oficializada e gravou na RAM[" + std::to_string(rawAddress) + "] o valor " + std::to_string(head.value));
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

void TomasuloSimulator::setRegisterValue(int regIndex, int value) {
    if (regIndex >= 0 && regIndex < registerFile.size()) {
        registerFile[regIndex] = value;
        Logger::log(Logger::INFO, "Registrador R" + std::to_string(regIndex) + " inicializado com o valor " + std::to_string(value));
    }
}

void TomasuloSimulator::setMemoryValue(int memAddress, int value) {
    if (memAddress >= 0 && memAddress < memory.size()) {
        memory[memAddress] = value;
        Logger::log(Logger::INFO, "Memoria RAM[" + std::to_string(memAddress) + "] inicializada com o valor " + std::to_string(value));
    } else {
        Logger::log(Logger::WARNING, "Endereco de memoria invalido.");
    }
}

int TomasuloSimulator::getRegistersSize() {
    return static_cast<int>(registerFile.size());
}

int TomasuloSimulator::getMemorySize() {
    return static_cast<int>(memory.size());
}

