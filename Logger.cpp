//---------------------------------------------------------------------------
#include "Logger.h"
#include <iostream>
#include <iomanip>

//---------------------------------------------------------------------------
Logger* Logger::instance = nullptr;

//---------------------------------------------------------------------------
Logger::Logger() : currentLevel(LogLevel::LOG_LEVEL_INFO), isInitialized(false) {
}

//---------------------------------------------------------------------------
Logger::~Logger() {
    close();
}

//---------------------------------------------------------------------------
Logger* Logger::getInstance() {
    if (instance == nullptr) {
        instance = new Logger();
    }
    return instance;
}

//---------------------------------------------------------------------------
bool Logger::initialize(const std::string& filePath, LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    try {
        // If file open, then close
        if (logFile.is_open()) {
            logFile.close();
        }
        
        // Create directory (simple method)
        size_t lastSlash = filePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            std::string dir = filePath.substr(0, lastSlash);
            // Directory creation logic (if needed)
        }
        
        // Open file (append mode)
        logFile.open(filePath, std::ios::app);
        if (!logFile.is_open()) {
            return false;
        }
        
        logFilePath = filePath;
        currentLevel = level;
        isInitialized = true;
        
        // Initialize log record
        info("Logger initialized successfully");
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Logger initialization failed: " << e.what() << std::endl;
        return false;
    }
}

//---------------------------------------------------------------------------
std::string Logger::getCurrentTimestamp() {
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    return std::string(buffer);
}

//---------------------------------------------------------------------------
std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LOG_LEVEL_DEBUG:    return "DEBUG";
        case LOG_LEVEL_INFO:     return "INFO";
        case LOG_LEVEL_WARNING:  return "WARNING";
        case LOG_LEVEL_ERROR:    return "ERROR";
        case LOG_LEVEL_CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

//---------------------------------------------------------------------------
void Logger::writeToFile(const LogEntry& entry) {
    if (!isInitialized || !logFile.is_open()) {
        return;
    }
    
    std::stringstream ss;
    ss << "[" << entry.timestamp << "] ";
    ss << "[" << levelToString(entry.level) << "] ";
    
    if (!entry.function.empty()) {
        ss << "[" << entry.function << "] ";
    }
    
    if (!entry.file.empty()) {
        std::filesystem::path path(entry.file);
        ss << "[" << path.filename().string();
        if (entry.line > 0) {
            ss << ":" << entry.line;
        }
        ss << "] ";
    }
    
    ss << entry.message << std::endl;
    
    logFile << ss.str();
    logFile.flush();
}

//---------------------------------------------------------------------------
void Logger::debug(const std::string& message, const std::string& function, 
                   const std::string& file, int line) {
    log(LogLevel::LOG_LEVEL_DEBUG, message, function, file, line);
}

//---------------------------------------------------------------------------
void Logger::info(const std::string& message, const std::string& function, 
                  const std::string& file, int line) {
    log(LogLevel::LOG_LEVEL_INFO, message, function, file, line);
}

//---------------------------------------------------------------------------
void Logger::warning(const std::string& message, const std::string& function, 
                     const std::string& file, int line) {
    log(LogLevel::LOG_LEVEL_WARNING, message, function, file, line);
}

//---------------------------------------------------------------------------
void Logger::error(const std::string& message, const std::string& function, 
                   const std::string& file, int line) {
    log(LogLevel::LOG_LEVEL_ERROR, message, function, file, line);
}

//---------------------------------------------------------------------------
void Logger::critical(const std::string& message, const std::string& function, 
                      const std::string& file, int line) {
    log(LogLevel::LOG_LEVEL_CRITICAL, message, function, file, line);
}

//---------------------------------------------------------------------------
void Logger::log(LogLevel level, const std::string& message, const std::string& function, 
                 const std::string& file, int line) {
    if (!isEnabled(level)) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex);
    
    LogEntry entry;
    entry.level = level;
    entry.timestamp = getCurrentTimestamp();
    entry.message = message;
    entry.function = function;
    entry.file = file;
    entry.line = line;
    
    writeToFile(entry);
    
    // ERROR와 CRITICAL 레벨은 콘솔에도 출력
    if (level >= LogLevel::LOG_LEVEL_ERROR) {
        std::cerr << "[" << levelToString(level) << "] " << message << std::endl;
    }
}

//---------------------------------------------------------------------------
void Logger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    currentLevel = level;
}

//---------------------------------------------------------------------------
void Logger::setLogFilePath(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(logMutex);
    logFilePath = filePath;
}

//---------------------------------------------------------------------------
bool Logger::isEnabled(LogLevel level) {
    return level >= currentLevel;
}

//---------------------------------------------------------------------------
void Logger::flush() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile.flush();
    }
}

//---------------------------------------------------------------------------
void Logger::close() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        info("Logger closing");
        logFile.close();
        isInitialized = false;
    }
}

//---------------------------------------------------------------------------
void Logger::setMaxFileSize(size_t maxSize) {
    // 구현 예정
}

//---------------------------------------------------------------------------
void Logger::rotateLogFile() {
    // 구현 예정
}