#include "logger.h"

std::shared_ptr<Logger> Logger::loggerInstance;


void Logger::setLogPreferences(std::string logFileName = "",
					   LogLevel level = LogLevel::ERROR,
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

void Logger::log(std::string codeFile, int codeLine, std::string message, LogLevel messageLevel = LogLevel::DEBUG) {
	if (messageLevel <= this->logLevel) {
		std::string logType;
		//Set Log Level Name
		switch (messageLevel) {
		case LogLevel::DEBUG:
			logType = "DEBUG: ";
			break;
		case LogLevel::INFO:
			logType = "INFO: ";
			break;
		case LogLevel::WARN:
			logType = "WARN: ";
			break;
		case LogLevel::ERROR:
			logType = "ERROR: ";
			break;
		default:
			logType = "NONE: ";
			break;
		}
		codeFile += " : " + std::to_string(codeLine) + " : ";
		message = logType + codeFile + message;

		logMessage(message);
	}
}

LogLevel Logger::setLogLevel(const std::string& logLevel) {
	if (logLevel == "DEBUG") {
		return LogLevel::DEBUG;
	}
	else if (logLevel == "INFO") {
		return LogLevel::INFO;
	}
	else if (logLevel == "WARN") {
		return LogLevel::ERROR;
	}
	else if (logLevel == "ERROR") {
		return LogLevel::ERROR;
	}
	return LogLevel::NONE;
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
	}
	else {
		std::cout << message << std::endl;
	}
}