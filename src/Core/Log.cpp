#include "Prefix.h"

#include "Log.h"

#include <cstdarg>
#include <cstdio>

#include <iostream>
#include <streambuf>
#include <ostream>
#include <string>
#include <mutex>

namespace gore
{

static Logger s_DefaultLogger;
#ifdef ENGINE_DEBUG
static LogLevel kDefaultLogLevel = LogLevel::DEBUG;
#else
static LogLevel kDefaultLogLevel = LogLevel::INFO;
#endif

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

LogStream Logger::Start(LogLevel level)
{
    return LogStream(level, this);
}

void Logger::Log(LogLevel level, const char* format, ...)
{
    if (level >= m_LogLevel)
    {
        std::string buf;

        va_list args1;
        va_start(args1, format);

        int sz = std::vsnprintf(nullptr, 0, format, args1);

        if (sz <= 0) buf = format;
        else
        {
            buf.resize(sz);

            va_list args2;
            va_copy(args2, args1);
            std::vsnprintf(&buf[0], sz + 1, format, args2);
            va_end(args2);
        }

        va_end(args1);


        std::lock_guard<std::mutex> lock(m_Mutex);
        *m_OutputStream << GetPrefix(level) << buf << std::flush;
    }
}

std::string Logger::GetPrefix(LogLevel level)
{
    switch (level)
    {
        case LogLevel::DEBUG:
            return "  DEBUG: ";
        case LogLevel::INFO:
            return "   INFO: ";
        case LogLevel::WARNING:
            return "WARNING: ";
        case LogLevel::ERROR:
            return "  ERROR: ";
        case LogLevel::FATAL:
            return "  FATAL: ";
        default:
            return "UNKNOWN: ";
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
        m_Stream->m_Logger->Log(m_Stream->m_LogLevel, m_BufferStream.str().c_str());
        m_BufferStream.str("");
        m_BufferStream.clear();
        return 0;
    }

private:
    LogStream* m_Stream;

    std::stringstream m_BufferStream; // Internal buffer to store log messages
};

LogStream::LogStream(LogLevel level, Logger* logger) :
    std::ostream(new LogBuffer(this)),
    m_Logger(logger),
    m_LogLevel(level)
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
