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

void exibirMenu(int& rsAdd, int& rsMul, int& rsLs, int& aluAdd, int& aluMul, int& aluLs, int& issueWidth, std::string& filename) {
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
    Logger::log(Logger::INFO,"7. Grau de Superescalaridade (Issue-N) (Padrao: 4): ");
    Logger::log(Logger::INFO, ">>> ", false); std::getline(std::cin, input); if (!input.empty()) { try { issueWidth = std::stoi(input); } catch(...) {} }
    Logger::log(Logger::INFO,"8. Selecione o arquivo de instrucoes:");
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
    Logger::log(Logger::INFO, std::to_string(aluAdd) + " ALUs ADD/SUB, " + std::to_string(aluMul)
        + " ALUs MUL/DIV, " + std::to_string(aluLs) + " ALUs LW/SW");
    Logger::log(Logger::INFO, "Issue/Band/Commit Width: " + std::to_string(issueWidth));
    Logger::log(Logger::INFO, "Lendo instrucoes de: " + filename);
    Logger::log(Logger::INFO, "========================================================");
    Logger::log(Logger::INFO,"Pressione ENTER para executar.");
    std::cin.get();
}

int main() {
    srand(42);
    try {
        int rsAdd = 2, rsMul = 1, rsLs = 1, aluAdd = 2, aluMul = 1, aluLs = 1, issueWidth = 2;
        std::string filename = "input.txt";
        exibirMenu(rsAdd, rsMul, rsLs, aluAdd, aluMul, aluLs, issueWidth, filename);
        TomasuloSimulator sim(rsAdd, rsMul, rsLs, aluAdd, aluMul, aluLs, issueWidth);
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