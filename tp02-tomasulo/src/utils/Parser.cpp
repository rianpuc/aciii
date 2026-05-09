#include "../../include/utils/Parser.hpp"
#include "../../include/utils/TomasuloException.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>

std::vector<Instruction> Parser::parseFile(const std::string& filename) {
    std::vector<Instruction> instructions;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw TomasuloException("Nao foi possivel abrir o arquivo: " + filename);
    }
    std::string line;
    int lineNumber = 0;
    int instructionId = 1;
    while (std::getline(file, line)) {
        lineNumber++;
        // checagem de comentario para remover
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        // limpando os espacos iniciais
        std::stringstream cleaner(line);
        std::string word;
        std::string cleanRaw = "";
        while (cleaner >> word) {
            cleanRaw += word + " ";
        }
        if (cleanRaw.empty()) continue;
        cleanRaw.pop_back();
        // transformando virgulas em espacos
        std::string parsingLine = cleanRaw;
        std::replace(parsingLine.begin(), parsingLine.end(), ',', ' ');

        std::stringstream ss(parsingLine);
        std::string opStr, arg1, arg2, arg3;
        // linha vazia
        if (!(ss >> opStr)) continue;

        Instruction inst;
        inst.id = instructionId++;
        inst.op = parseOpcode(opStr, lineNumber);
        inst.rawText = cleanRaw;

        if (inst.op == ADD || inst.op == SUB || inst.op == MUL || inst.op == DIV) {
            inst.type = TYPE_R;
            if (!(ss >> arg1 >> arg2 >> arg3)) {
                throw TomasuloException("Erro na linha " + std::to_string(lineNumber) + ": Tipo R exige 3 registradores.");
            }
            inst.destRegister = parseRegister(arg1, lineNumber);
            inst.srcRegister1 = parseRegister(arg2, lineNumber);
            inst.srcRegister2 = parseRegister(arg3, lineNumber);
            inst.immediate = 0;
        } else if (inst.op == LW || inst.op == SW) {
            inst.type = TYPE_I;
            if (!(ss >> arg1 >> arg2)) {
                throw TomasuloException("Erro na linha " + std::to_string(lineNumber) + ": Tipo I exige Registrador e Offset. Ex: LW R1, 100(R2)");
            }
            inst.destRegister = parseRegister(arg1, lineNumber);
            size_t inicioParenteses = arg2.find('(');
            size_t fimParenteses = arg2.find(')');
            if (inicioParenteses == std::string::npos || fimParenteses == std::string::npos) {
                throw TomasuloException("Sintaxe de memoria invalida na linha " + std::to_string(lineNumber) + ". Use offset(reg).");
            }
            std::string offsetStr = arg2.substr(0, inicioParenteses);
            inst.immediate = std::stoi(offsetStr);
            std::string baseRegStr = arg2.substr(inicioParenteses + 1, fimParenteses - inicioParenteses - 1);
            inst.srcRegister1 = parseRegister(baseRegStr, lineNumber);
            inst.srcRegister2 = -1;
        }
        instructions.push_back(inst);
    }
    file.close();
    return instructions;
}

Opcode Parser::parseOpcode(const std::string& opStr, int lineNumber) {
    if (opStr == "ADD") return ADD;
    if (opStr == "SUB") return SUB;
    if (opStr == "MUL") return MUL;
    if (opStr == "DIV") return DIV;
    if (opStr == "LW") return LW;
    if (opStr == "SW") return SW;
    
    throw TomasuloException("Opcode desconhecido '" + opStr + "' na linha " + std::to_string(lineNumber));
}

int Parser::parseRegister(const std::string& regStr, int lineNumber) {
    if (regStr.empty() || regStr[0] != 'R') {
        throw TomasuloException("Registrador invalido '" + regStr + "' na linha " + std::to_string(lineNumber) + ". Use formato R0, R1...");
    }

    try {
        int registerNumber = std::stoi(regStr.substr(1));
        if (registerNumber < 0 || registerNumber > 31) {
            throw TomasuloException("Registrador fora dos limites (" + regStr + ") na linha " + std::to_string(lineNumber) + ". O hardware suporta apenas de R0 a R31.");
        }
        return registerNumber;
    } catch (const TomasuloException& e) {
        throw;
    } catch (...) {
        throw TomasuloException("Falha ao ler numero do registrador '" + regStr + "' na linha " + std::to_string(lineNumber));
    }
}