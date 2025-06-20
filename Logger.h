//---------------------------------------------------------------------------
#ifndef LoggerH
#define LoggerH

#include <vcl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <mutex>

// 로그 레벨 정의 (C++Builder 호환 버전)
enum LogLevel {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARNING = 2,
    LOG_LEVEL_ERROR = 3,
    LOG_LEVEL_CRITICAL = 4
};

// 로그 엔트리 구조체
struct LogEntry {
    LogLevel level;
    std::string timestamp;
    std::string message;
    std::string function;
    std::string file;
    int line;
};

//---------------------------------------------------------------------------
class Logger {
private:
    static Logger* instance;
    std::ofstream logFile;
    std::string logFilePath;
    LogLevel currentLevel;
    bool isInitialized;
    std::mutex logMutex;
    
    // 싱글톤 패턴을 위한 private 생성자
    Logger();
    ~Logger();
    
    // 복사 생성자와 대입 연산자 방지
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::string getCurrentTimestamp();
    std::string levelToString(LogLevel level);
    void writeToFile(const LogEntry& entry);
    
public:
    // 싱글톤 인스턴스 반환
    static Logger* getInstance();
    
    // 초기화
    bool initialize(const std::string& filePath, LogLevel level = LogLevel::LOG_LEVEL_INFO);
    
    // 로그 기록 메서드들
    void debug(const std::string& message, const std::string& function = "", const std::string& file = "", int line = 0);
    void info(const std::string& message, const std::string& function = "", const std::string& file = "", int line = 0);
    void warning(const std::string& message, const std::string& function = "", const std::string& file = "", int line = 0);
    void error(const std::string& message, const std::string& function = "",const std::string& file = "", int line = 0);
    void critical(const std::string& message, const std::string& function = "", const std::string& file = "", int line = 0);

    // 범용 로그 메서드
    void log(LogLevel level, const std::string& message, const std::string& function = "", 
             const std::string& file = "", int line = 0);
    
    // 설정 변경
    void setLogLevel(LogLevel level);
    void setLogFilePath(const std::string& filePath);
    
    // 유틸리티
    bool isEnabled(LogLevel level);
    void flush();
    void close();
    
    // 로그 파일 크기 제한 및 로테이션
    void setMaxFileSize(size_t maxSize);
    void rotateLogFile();
};

// 매크로를 통한 편의 함수들 (C++Builder 호환 버전)
//#define LOG_DEBUG(msg) Logger::getInstance()->debug(msg, "", __FILE__, __LINE__)
//#define LOG_INFO(msg) Logger::getInstance()->info(msg, "", __FILE__, __LINE__)
//#define LOG_WARNING(msg) Logger::getInstance()->warning(msg, "", __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::getInstance()->error(msg, "", __FILE__, __LINE__)
//#define LOG_CRITICAL(msg) Logger::getInstance()->critical(msg, "", __FILE__, __LINE__)

// 함수명을 포함하는 매크로 (선택적)
//#define LOG_DEBUG_FUNC(msg, func) Logger::getInstance()->debug(msg, func, __FILE__, __LINE__)
//#define LOG_INFO_FUNC(msg, func) Logger::getInstance()->info(msg, func, __FILE__, __LINE__)
//#define LOG_WARNING_FUNC(msg, func) Logger::getInstance()->warning(msg, func, __FILE__, __LINE__)
//#define LOG_ERROR_FUNC(msg, func) Logger::getInstance()->error(msg, func, __FILE__, __LINE__)
//#define LOG_CRITICAL_FUNC(msg, func) Logger::getInstance()->critical(msg, func, __FILE__, __LINE__)

//---------------------------------------------------------------------------
#endif