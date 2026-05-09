#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <string>
#include "../core/Instruction.hpp"

class Parser {
public:
    static std::vector<Instruction> parseFile(const std::string& filename);

private:
    static Opcode parseOpcode(const std::string& opStr, int lineNumber);
    static int parseRegister(const std::string& regStr, int lineNumber);
};

#endif