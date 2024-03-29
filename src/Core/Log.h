#pragma once

#include "Export.h"

#include <ostream>
#include <sstream>
#include <streambuf>
#include <mutex>

// usage: LOG_STREAM(LEVEL) << "Hello" << std::endl << "World!" << std::endl;
//    or: LOG(LEVEL, "Hello %s\n", "World!");

#define LOG_STREAM(LEVEL) (::gore::Logger::Default().StartStream(::gore::LogLevel::LEVEL, __FILE__, __LINE__))
#define LOG(LEVEL, ...)                                                      \
    do {                                                                     \
        ::gore::Logger::Default().Log(::gore::LogLevel::LEVEL, __FILE__, __LINE__, __VA_ARGS__); \
    } while (false)

namespace gore
{

enum class LogLevel : uint8_t
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
};

class Logger;

ENGINE_CLASS(LogStream) final : public std::ostream
{
public:
    LogStream(LogLevel level, const char* file, int line, Logger* logger);
    ~LogStream() override;

    NON_COPYABLE(LogStream);

private:
    friend class LogBuffer;
    Logger* m_Logger;
    LogLevel m_LogLevel;

    const char* m_File;
    int m_Line;

    [[nodiscard]] bool ShouldLog() const;
};

ENGINE_CLASS(Logger)
{
public:
    Logger();
    ~Logger();

    NON_COPYABLE(Logger);

    static Logger& Default();

    LogStream StartStream(LogLevel level, const char* file, int line);

    void Log(LogLevel level, const char* file, int line, const char* format, ...);

    [[nodiscard]] LogLevel GetLevel() const
    {
        return m_LogLevel;
    }
    void SetLevel(LogLevel level)
    {
        m_LogLevel = level;
    }

private:
    bool m_OwnsStream;
    std::ostream* m_OutputStream;

    LogLevel m_LogLevel;
    std::mutex m_Mutex;
};

} // namespace gore
