#include <iostream>
#include <vector>

#include "../include/utils/Logger.hpp"
#include "../include/utils/Parser.hpp"
#include "../include/utils/TomasuloException.hpp"

int main() {
    try {
        Logger::log(Logger::INFO, "Inicializando o simulador...");
        std::vector<Instruction> instructions = Parser::parseFile("input.txt");
        int cycle = 0;
        for (auto& instruction : instructions) {
            Logger::log(cycle++,Logger::DEBUG, instruction.toString());
        }
    } catch (const TomasuloException& e) {
        Logger::log(Logger::ERROR, e.what());
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Erro fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}