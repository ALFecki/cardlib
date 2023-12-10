#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

enum class LogLvl { NONE = 0, ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4 };

enum class LogOutput { CONSOLE, FILE };

class Logger {
public:
    static std::shared_ptr<Logger> getInstance();

    void setLogPreferences(std::string logFileName, LogLvl level, LogOutput output);

    void log(std::string codeFile, int codeLine, std::string message, LogLvl messageLevel);

    LogOutput setLogOutput(const std::string& logOutput);
    LogLvl setLogLevel(const std::string& logLevel);

private:
    LogLvl logLevel;
    LogOutput logOutput = LogOutput::CONSOLE;
    std::ofstream logFile;

    static std::shared_ptr<Logger> loggerInstance;

    void logMessage(const std::string& message);
};

// auto logger = Logger::getInstance();

#endif