#include "CommonLogger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

std::ofstream CommonLogger::m_fileStream;
std::mutex CommonLogger::m_mutex;

void CommonLogger::init(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fileStream.is_open()) m_fileStream.close();
    m_fileStream.open(filePath, std::ios::app);
}

CommonLogger::CommonLogger(LogLevel level, const char* file, const char* func, int line)
    : m_level(level), m_file(file), m_func(func), m_line(line) {
}

// 👇 这是真正执行写入硬盘的私有函数
void CommonLogger::outputToFile(const std::string& userMsg) {
    std::ostringstream finalLog;
    finalLog << "[" << getTimestamp() << "] "
        << "[" << getLevelString() << "] "
        << "[" << extractFileName(m_file) << ":" << m_line << "] "
        << "[" << m_func << "] "
        << userMsg << "\n"; // 拼上刚才 format 替换好的用户内容

    std::string logStr = finalLog.str();

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fileStream.is_open()) {
        m_fileStream << logStr;
        m_fileStream.flush();
    }
    std::cout << logStr; // 控制台也打印一份
}

// ... 下面的 getTimestamp, getLevelString, extractFileName 保持原样不变 ...

// 获取当前时间字符串 (例如：2026-02-17 10:30:15.123)
std::string CommonLogger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto timer = std::chrono::system_clock::to_time_t(now);
    std::tm bt{};
#if defined(_MSC_VER)
    localtime_s(&bt, &timer); // Windows 安全版本
#else
    localtime_r(&timer, &bt); // Linux 版本
#endif
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string CommonLogger::getLevelString() const {
    switch (m_level) {
    case LogLevel::INFO:    return "INFO ";
    case LogLevel::WARNING: return "WARN ";
    case LogLevel::ERROR:   return "ERROR";
    default:                return "UNKNOWN";
    }
}

// 提取纯文件名（去掉前面冗长的 E:/Code/... 路径）
std::string CommonLogger::extractFileName(const char* path) const {
    std::string fullPath(path);
    size_t pos = fullPath.find_last_of("/\\");
    return (pos == std::string::npos) ? fullPath : fullPath.substr(pos + 1);
}