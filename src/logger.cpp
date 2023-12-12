#include "logger.h"

std::shared_ptr<Logger> Logger::loggerInstance;

// auto logger = Logger::getInstance();

void Logger::setLogPreferences(std::string logFileName = "",
                               LogLvl level = LogLvl::ERROR,
                               LogOutput output = LogOutput::CONSOLE) {
    this->logLevel = level;
    this->logOutput = output;

    if (logOutput == LogOutput::FILE && !logFileName.empty()) {
        logFile.open(logFileName);
        if (!logFile.good()) {
            std::cerr << "Can't Open Log File" << std::endl;
            logOutput = LogOutput::CONSOLE;
        }
    }
}

std::shared_ptr<Logger> Logger::getInstance() {
    if (loggerInstance == nullptr) {
        loggerInstance = std::shared_ptr<Logger>(new Logger());
    }

    return loggerInstance;
}

void Logger::log(std::string codeFile,
                 int codeLine,
                 std::string message,
                 LogLvl messageLevel = LogLvl::DEBUG) {
    std::string logType;
    std::string colorCode;
    std::string resetColorCode = "\033[0m";
    // Set Log Level Name
    switch (messageLevel) {
        case LogLvl::DEBUG:
            logType = "DEBUG: ";
            colorCode = "\033[1;34m";
            break;
        case LogLvl::INFO:
            logType = "INFO: ";
            colorCode = "\033[1;32m";
            break;
        case LogLvl::WARN:
            logType = "WARN: ";
            colorCode = "\033[1;33m";
            break;
        case LogLvl::ERROR:
            logType = "ERROR: ";
            colorCode = "\033[1;31m";
            break;
        default:
            logType = "NONE: ";
            colorCode = "\033[0m";
            break;
    }
    codeFile += " : " + std::to_string(codeLine) + " : ";
    message = colorCode + logType + resetColorCode + codeFile + message;

    logMessage(message);
}

LogLvl Logger::setLogLevel(const std::string& logLevel) {
    if (logLevel == "DEBUG") {
        return LogLvl::DEBUG;
    } else if (logLevel == "INFO") {
        return LogLvl::INFO;
    } else if (logLevel == "WARN") {
        return LogLvl::ERROR;
    } else if (logLevel == "ERROR") {
        return LogLvl::ERROR;
    }
    return LogLvl::NONE;
}

LogOutput Logger::setLogOutput(const std::string& logOutput) {
    if (logOutput == "FILE") {
        return LogOutput::FILE;
    }

    return LogOutput::CONSOLE;
}

void Logger::logMessage(const std::string& message) {
    if (logOutput == LogOutput::FILE) {
        logFile << message << std::endl;
    } else {
        std::cout << message << std::endl;
    }
}