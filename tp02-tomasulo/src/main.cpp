#include <iostream>
#include "../include/core/TomasuloSimulator.hpp"
#include "../include/utils/Logger.hpp"
#include "../include/utils/Parser.hpp"
#include "../include/utils/TomasuloException.hpp"

int main() {
    try {
        TomasuloSimulator tomasulo = TomasuloSimulator();
        tomasulo.run("input.txt");
    } catch (const TomasuloException& e) {
        Logger::log(Logger::ERROR, e.what());
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Erro fatal: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}