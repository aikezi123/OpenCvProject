#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <format> // 核心：引入 C++20 的 format 库

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class CommonLogger {
public:
    CommonLogger(LogLevel level, const char* file, const char* func, int line);
    ~CommonLogger() = default;

    // 全局初始化
    static void init(const std::string& filePath);

    // 👇 【核心黑魔法】：接受任意数量参数的模板函数
    template <typename... Args>
    void write(std::string_view fmt, Args&&... args) {
        // 1. 使用 C++20 的标准格式化，把 "{}" 替换成真实变量
        std::string userMsg = std::vformat(fmt, std::make_format_args(args...));

        // 2. 交给底层去拼上时间、行号，并写入文件
        outputToFile(userMsg);
    }

private:
    LogLevel m_level;
    const char* m_file;
    const char* m_func;
    int m_line;

    static std::ofstream m_fileStream;
    static std::mutex m_mutex;

    void outputToFile(const std::string& userMsg);
    std::string getLevelString() const;
    std::string getTimestamp() const;
    std::string extractFileName(const char* path) const;
};

// 👇 【宏定义升级】：使用 __VA_ARGS__ 接收不定长参数并传给 write 函数
#define LOG_INFO(...)    CommonLogger(LogLevel::INFO, __FILE__, __func__, __LINE__).write(__VA_ARGS__)
#define LOG_WARNING(...) CommonLogger(LogLevel::WARNING, __FILE__, __func__, __LINE__).write(__VA_ARGS__)
#define LOG_ERROR(...)   CommonLogger(LogLevel::ERROR, __FILE__, __func__, __LINE__).write(__VA_ARGS__)