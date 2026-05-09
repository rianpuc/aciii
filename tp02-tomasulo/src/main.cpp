#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include "../include/core/TomasuloSimulator.hpp"
#include "../include/utils/Logger.hpp"
#include "../include/utils/TomasuloException.hpp"

namespace fs = std::filesystem;

void exibirMenu(int& numAdd, int& numMul, int& numLs, int& issueWidth, std::string& filename) {
    std::string input;
    std::cout << "\n========================================================\n";
    std::cout << "          SIMULADOR TOMASULO\n";
    std::cout << "========================================================\n";
    std::cout << "Pressione ENTER para manter os valores padrao.\n\n";
    std::cout << "1. Quantidade de Estacoes ADD/SUB (Padrao: 2): ";
    std::getline(std::cin, input);
    if (!input.empty()) { try { numAdd = std::stoi(input); } catch(...) {} }
    std::cout << "2. Quantidade de Estacoes MUL/DIV (Padrao: 1): ";
    std::getline(std::cin, input);
    if (!input.empty()) { try { numMul = std::stoi(input); } catch(...) {} }
    std::cout << "3. Quantidade de Estacoes LW/SW   (Padrao: 2): ";
    std::getline(std::cin, input);
    if (!input.empty()) { try { numLs = std::stoi(input); } catch(...) {} }
    std::cout << "4. Grau de Superescalaridade (Issue-N) (Padrao: 2): ";
    std::getline(std::cin, input);
    if (!input.empty()) { try { issueWidth = std::stoi(input); } catch(...) {} }
    std::cout << "\n5. Selecione o arquivo de instrucoes:\n";
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
            std::cout << "   [" << i + 1 << "] " << cleanPath << "\n";
        }
        std::cout << "   [0] Digitar o caminho do arquivo manualmente\n";
        std::cout << "   Escolha (Padrao: 1): ";

        std::getline(std::cin, input);
        if (input.empty()) {
            filename = txtFiles[0];
        } else {
            try {
                int choice = std::stoi(input);
                if (choice == 0) {
                    std::cout << "   Digite o caminho completo: ";
                    std::getline(std::cin, filename);
                } else if (choice > 0 && choice <= (int)txtFiles.size()) {
                    filename = txtFiles[choice - 1];
                } else {
                    std::cout << "   Opcao invalida. Usando padrao [1].\n";
                    filename = txtFiles[0];
                }
            } catch (...) {
                std::cout << "   Entrada invalida. Usando padrao [1].\n";
                filename = txtFiles[0];
            }
        }
    } else {
        std::cout << "   (Nenhum arquivo .txt encontrado na pasta 'tests/'.)\n";
        std::cout << "   Digite o caminho manualmente (Padrao: codigo.txt): ";
        std::getline(std::cin, input);
        if (!input.empty()) filename = input;
    }

    std::cout << "========================================================\n";
    Logger::log(Logger::INFO, "Inicializando simulacao com " + std::to_string(numAdd) + " ADD/SUB, " + std::to_string(numMul)
        + " MUL/DIV, " + std::to_string(numLs) + " LW/SW | Issue Width: " + std::to_string(issueWidth));
    Logger::log(Logger::INFO, "Lendo instrucoes de: " + filename);
    std::cout << "========================================================\n";
    std::cout << "Pressione ENTER para executar.\n";
    std::cin.get();
}

int main() {
    try {
        int numAdd = 2, numMul = 1, numLs = 2, issueWidth = 2;
        std::string filename = "input.txt";
        exibirMenu(numAdd, numMul, numLs, issueWidth, filename);
        TomasuloSimulator sim(numAdd, numMul, numLs, issueWidth);
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