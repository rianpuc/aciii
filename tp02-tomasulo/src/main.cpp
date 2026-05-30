#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include "../include/core/TomasuloSimulator.hpp"
#include "../include/utils/Logger.hpp"
#include "../include/utils/TomasuloException.hpp"

namespace fs = std::filesystem;

void exibirMenu(int& rsAdd, int& rsMul, int& rsLs, int& aluAdd, int& aluMul, int& aluLs, int& cdbWidth, int& latAdd, int& latMul, int& latLs, std::string& filename) {
    std::string input;
    Logger::log(Logger::INFO, "========================================================");
    Logger::log(Logger::INFO, "             SIMULADOR TOMASULO");
    Logger::log(Logger::INFO, "========================================================");
    Logger::log(Logger::INFO, "Pressione ENTER para manter os valores padrao.");
    Logger::log(Logger::INFO, "1. Quantidade de Estacoes ADD/SUB (Padrao: 2): ");
    Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input);
    if (!input.empty()) { try { rsAdd = std::stoi(input); } catch(...) {} }
    Logger::log(Logger::INFO,"2. Quantidade de Estacoes MUL/DIV (Padrao: 1): ");
    Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input);
    if (!input.empty()) { try { rsMul = std::stoi(input); } catch(...) {} }
    Logger::log(Logger::INFO,"3. Quantidade de Estacoes LW/SW   (Padrao: 1): ");
    Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input);
    if (!input.empty()) { try { rsLs = std::stoi(input); } catch(...) {} }
    Logger::log(Logger::INFO,"--- UNIDADES FISICAS DE EXECUCAO (ALUs) ---");
    Logger::log(Logger::INFO,"4. Somadores ADD/SUB (Padrao: 2): ");
    Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input); if (!input.empty()) { try { aluAdd = std::stoi(input); } catch (...) {} }
    Logger::log(Logger::INFO,"5. Multiplicadores MUL/DIV (Padrao: 1): ");
    Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input); if (!input.empty()) { try { aluMul = std::stoi(input); } catch (...) {} }
    Logger::log(Logger::INFO,"6. Unidades de Memoria LW/SW (Padrao: 1): ");
    Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input); if (!input.empty()) { try { aluLs = std::stoi(input); } catch (...) {} }
    Logger::log(Logger::INFO,"--- PIPELINE ---");
    Logger::log(Logger::INFO,"7. Largura do Common Data Bus (CDB) (Padrao: 1): ");
    Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input); if (!input.empty()) { try { cdbWidth = std::stoi(input); } catch(...) {} }
    Logger::log(Logger::INFO,"--- LATENCIA DE EXECUCAO (Ciclos) ---");
    Logger::log(Logger::INFO,"8. Ciclos para ADD/SUB (Padrao: 2): ");
    Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input); if (!input.empty()) { try { latAdd = std::stoi(input); } catch (...) {} }
    Logger::log(Logger::INFO,"9. Ciclos para MUL/DIV (Padrao: 10): ");
    Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input); if (!input.empty()) { try { latMul = std::stoi(input); } catch (...) {} }
    Logger::log(Logger::INFO,"10. Ciclos para LW/SW (Padrao: 3): ");
    Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input); if (!input.empty()) { try { latLs = std::stoi(input); } catch (...) {} }
    Logger::log(Logger::INFO,"11. Selecione o arquivo de instrucoes:");
    std::vector<std::string> txtFiles;
    std::string folderPath = "tests";
    if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                txtFiles.push_back(entry.path().string());
            }
        }
    }
    if (!txtFiles.empty()) {
        for (size_t i = 0; i < txtFiles.size(); ++i) {
            std::string cleanPath = txtFiles[i];
            std::replace(cleanPath.begin(), cleanPath.end(), '\\', '/');
            Logger::log(Logger::INFO, "   [" + std::to_string(i + 1) + "] " + cleanPath);
        }
        Logger::log(Logger::INFO,"   [0] Digitar o caminho do arquivo manualmente");
        Logger::log(Logger::INFO,"   Escolha (Padrao: 1): ");
        Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input);
        if (input.empty()) {
            filename = txtFiles[0];
        } else {
            try {
                int choice = std::stoi(input);
                if (choice == 0) {
                    Logger::log(Logger::INFO, "   Digite o caminho completo: ");
                    Logger::log(Logger::INFO, ">>>", false); std::getline(std::cin, filename);
                } else if (choice > 0 && choice <= (int)txtFiles.size()) {
                    filename = txtFiles[choice - 1];
                } else {
                    Logger::log(Logger::WARNING,"   Opcao invalida. Usando padrao [1].\n");
                    filename = txtFiles[0];
                }
            } catch (...) {
                Logger::log(Logger::ERROR, "   Entrada invalida. Usando padrao [1].\n");
                filename = txtFiles[0];
            }
        }
    } else {
        Logger::log(Logger::WARNING, "   (Nenhum arquivo .txt encontrado na pasta 'tests/'.)\n");
        Logger::log(Logger::INFO,"   Digite o caminho manualmente (Padrao: codigo.txt): ");
        std::getline(std::cin, input);
        if (!input.empty()) filename = input;
    }
    Logger::log(Logger::INFO, "========================================================");
    Logger::log(Logger::INFO, "Inicializando simulacao com " + std::to_string(rsAdd) + " Estacoes ADD/SUB, " + std::to_string(rsMul)
        + " Estacoes MUL/DIV, " + std::to_string(rsLs) + " Estacoes LW/SW");
    Logger::log(Logger::INFO, std::to_string(aluAdd) + " ALUs ADD/SUB (Delay " + std::to_string(latAdd) + " ciclos), "
        + std::to_string(aluMul) + " ALUs MUL/DIV (Delay " + std::to_string(latMul) + " ciclos), "
        + std::to_string(aluLs) + " ALUs LW/SW (Delay " + std::to_string(latLs) + " ciclos)");
    Logger::log(Logger::INFO, "CDB Width: " + std::to_string(cdbWidth));
    Logger::log(Logger::INFO, "Lendo instrucoes de: " + filename);
    Logger::log(Logger::INFO, "========================================================");
    Logger::log(Logger::INFO,"Pressione ENTER para continuar.");
    std::cin.get();
}

void preSimMenu(TomasuloSimulator& sim) {
    Logger::log(Logger::INFO, "========================================================");
    Logger::log(Logger::INFO, "   MENU DE INJECAO DE ESTADO (PRE-SIMULACAO)");
    Logger::log(Logger::INFO, "========================================================");
    Logger::log(Logger::INFO, "1. Iniciar Simulacao agora");
    Logger::log(Logger::INFO, "2. Forcar valor em um Registrador (R0 - R" + std::to_string(sim.getRegistersSize() - 1) + ")");
    Logger::log(Logger::INFO, "3. Forcar valor na Memoria RAM  (0 - " + std::to_string(sim.getMemorySize() - 1) + ")");
    Logger::log(Logger::INFO, ">>> Escolha: ", false);
}

int main() {
    srand(42);
    try {
        int rsAdd = 2, rsMul = 1, rsLs = 1, aluAdd = 2, aluMul = 1, aluLs = 1, cdbWidth = 1, latAdd = 2, latMul = 10, latLs = 3;
        std::string filename = "cenario1.txt";
        exibirMenu(rsAdd, rsMul, rsLs, aluAdd, aluMul, aluLs, cdbWidth, latAdd, latMul, latLs, filename);
        TomasuloSimulator sim(rsAdd, rsMul, rsLs, aluAdd, aluMul, aluLs, cdbWidth, latAdd, latMul, latLs);
        bool startSimulation = false;
        while (!startSimulation) {
            preSimMenu(sim);
            std::string escolhaStr;
            std::getline(std::cin, escolhaStr);
            int escolha = 0;
            try { escolha = std::stoi(escolhaStr); } catch (...) {}
            switch (escolha) {
                case 1:
                    startSimulation = true;
                    break;
                case 2: {
                    int reg = -1, val = 0;
                    Logger::log(Logger::INFO, "Qual Registrador (0 a " + std::to_string(sim.getRegistersSize()-1) + ")? ", false);
                    std::cin >> reg;
                    Logger::log(Logger::INFO, "Qual o valor? ", false);
                    std::cin >> val;
                    std::cin.ignore(10000, '\n');
                    if (reg >= 0 && reg < 32) {
                        sim.setRegisterValue(reg, val);
                    } else {
                        Logger::log(Logger::ERROR, "Registrador invalido!");
                    }
                    break;
                }
                case 3: {
                    int addr = -1, val = 0;
                    Logger::log(Logger::INFO, "Qual Endereco da RAM (0 a " + std::to_string(sim.getMemorySize() - 1) + ")? ",  false);
                    std::cin >> addr;
                    Logger::log(Logger::INFO, "Qual o valor? ", false);
                    std::cin >> val;
                    std::cin.ignore(10000, '\n');
                    if (addr >= 0 && addr < 1024) {
                        sim.setMemoryValue(addr, val);
                    } else {
                        Logger::log(Logger::ERROR, "Endereco invalido!");
                    }
                    break;
                }
                default:
                    Logger::log(Logger::WARNING, "Opcao invalida. Digite 1, 2 ou 3.");
                    break;
            }
        }
        sim.run(filename);
    } catch (const TomasuloException& e) {
        Logger::log(Logger::ERROR, e.what());
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Erro fatal: " << e.what() << std::endl;
        return 1;
    }
    Logger::log(Logger::INFO, "Finalizando programa com sucesso.");
    return 0;
}