#include "../../include/utils/Logger.hpp"
#include <iostream>

std::string Logger::getLevelStr(const Logger::Level level) {
    switch (level) {
        case INFO:    return "\033[36m[INFO]\033[0m  ";
        case WARNING: return "\033[33m[WARN]\033[0m  ";
        case ERROR:   return "\033[31m[ERROR]\033[0m ";
        case DEBUG:   return "\033[35m[DEBUG]\033[0m ";
        default: return "\033[90m[UNKNOWN]\033[0m ";
    }
}

void Logger::log(const int cycle, const Logger::Level level, const std::string& message) {
    std::cout << "[Ciclo " << cycle << "] ";
    log(level, message);
}

void Logger::log(const Logger::Level level, const std::string& message) {
    log(level, message, true);
}

void Logger::log(const Logger::Level level, const std::string& message, bool breakLine) {
    const std::string levelStr = getLevelStr(level);
    if (breakLine) std::cout << levelStr << message << std::endl;
    else std::cout << levelStr << message;
}