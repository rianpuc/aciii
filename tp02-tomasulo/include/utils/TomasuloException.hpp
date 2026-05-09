#ifndef TOMASULO_EXCEPTION_HPP
#define TOMASULO_EXCEPTION_HPP

#include <stdexcept>
#include <string>

class TomasuloException : public std::runtime_error {
public:
    explicit TomasuloException(const std::string& msg) 
        : std::runtime_error("Erro no Simulador: " + msg) {}
};

#endif