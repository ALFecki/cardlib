#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <memory>
#include <fstream>

enum class LogLevel {
	NONE = 0,
	ERROR = 1,
	WARN = 2,
	INFO = 3,
	DEBUG = 4
};

enum class LogOutput {
	CONSOLE,
	FILE
};

class Logger {
public:
	static std::shared_ptr<Logger> getInstance();

	void setLogPreferences(std::string logFileName,
						   LogLevel level,
						   LogOutput output);

	void log(std::string codeFile, int codeLine, std::string message, LogLevel messageLevel);

	LogOutput setLogOutput(const std::string& logOutput);
	LogLevel setLogLevel(const std::string& logLevel);

private:
	LogLevel logLevel;
	LogOutput logOutput = LogOutput::CONSOLE;
	std::ofstream logFile;

	static std::shared_ptr<Logger> loggerInstance;

	void logMessage(const std::string& message);

};

#endif