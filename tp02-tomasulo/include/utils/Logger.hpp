#ifndef ACIII_LOGGER_H
#define ACIII_LOGGER_H
#include <string>

class Logger {
public:
    enum Level { INFO, WARNING, ERROR, DEBUG };
    static std::string getLevelStr(Level level);
    static void log(int cycle, Level level, const std::string& message);
    static void log(Level level, const std::string& message);
    static void log(Level level, const std::string& message, bool breakLine);
};

#endif