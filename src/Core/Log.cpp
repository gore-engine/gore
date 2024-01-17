#include "Prefix.h"
#define _CRT_SECURE_NO_WARNINGS

#include "Log.h"

#include <cstdarg>
#include <cstdio>
#include <ctime>

#include <iostream>
#include <iomanip>
#include <streambuf>
#include <ostream>
#include <string>
#include <mutex>
#include <chrono>
#include <filesystem>

namespace gore
{

static Logger s_DefaultLogger;
#ifdef ENGINE_DEBUG
static LogLevel kDefaultLogLevel = LogLevel::DEBUG;
#else
static LogLevel kDefaultLogLevel = LogLevel::INFO;
#endif

std::string GetLogLevelStr(LogLevel level)
{
    switch (level)
    {
        case LogLevel::DEBUG:
            return "  DEBUG";
        case LogLevel::INFO:
            return "   INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "  ERROR";
        case LogLevel::FATAL:
            return "  FATAL";
        default:
            return "UNKNOWN";
    }
}

Logger::Logger() :
    m_LogLevel(kDefaultLogLevel),
    m_OwnsStream(false),
    m_OutputStream(&std::cout)
{
}

Logger::~Logger()
{
    if (m_OwnsStream)
        delete m_OutputStream;
}

Logger& Logger::Default()
{
    return s_DefaultLogger;
}

LogStream Logger::StartStream(LogLevel level, const char* file, int line)
{
    return LogStream(level, file, line, this);
}

void Logger::Log(LogLevel level, const char* file, int line, const char* format, ...)
{
    if (level >= m_LogLevel)
    {
        auto now       = std::chrono::system_clock::now();
        auto nowTime   = std::chrono::system_clock::to_time_t(now);
        auto nowTimeTm = std::localtime(&nowTime);
        auto nowMs     = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto nowMsTime = nowMs.time_since_epoch().count() % 1000;

        std::string buf;

        va_list args1;
        va_start(args1, format);

        va_list args2;
        va_copy(args2, args1);

        int sz = std::vsnprintf(nullptr, 0, format, args1);

        if (sz <= 0)
            buf = format;
        else
        {
            buf.resize(sz);

            std::vsnprintf(&buf[0], sz + 1, format, args2);
        }

        va_end(args1);


        std::lock_guard<std::mutex> lock(m_Mutex);
        *m_OutputStream << std::put_time(nowTimeTm, "%Y-%m-%d %H:%M:%S")
                        << "." << std::setfill('0') << std::setw(3) << nowMsTime
                        << " " << GetLogLevelStr(level) << ": " << buf;

        if (level >= LogLevel::WARNING)
            *m_OutputStream << "    " << std::filesystem::weakly_canonical(file).make_preferred().string() << ":" << line << std::endl;
        else
            *m_OutputStream << std::flush;
    }
}

class LogBuffer final : public std::streambuf
{
public:
    explicit LogBuffer(LogStream* stream) :
        m_Stream(stream)
    {
    }

    ~LogBuffer() override
    {
        if (!m_BufferStream.str().empty())
            sync();
    }

protected:
    // Override overflow to handle characters
    int_type overflow(int_type c) override
    {
        if (c != EOF && m_Stream->ShouldLog())
        {
            // Append character to the internal buffer
            m_BufferStream << static_cast<char>(c);
        }
        return c;
    }

    // Sync is called on flush (e.g., by std::endl)
    int sync() override
    {
        if (!m_Stream->ShouldLog())
            return 0;

        m_BufferStream.flush();
        m_Stream->m_Logger->Log(m_Stream->m_LogLevel, m_Stream->m_File, m_Stream->m_Line, m_BufferStream.str().c_str());
        m_BufferStream.str("");
        m_BufferStream.clear();
        return 0;
    }

private:
    LogStream* m_Stream;

    std::stringstream m_BufferStream; // Internal buffer to store log messages
};

LogStream::LogStream(LogLevel level, const char* file, int line, Logger* logger) :
    std::ostream(new LogBuffer(this)),
    m_Logger(logger),
    m_LogLevel(level),
    m_File(file),
    m_Line(line)
{
}

LogStream::~LogStream()
{
    delete rdbuf();
}

bool LogStream::ShouldLog() const
{
    return m_LogLevel >= m_Logger->GetLevel();
}

} // namespace gore
