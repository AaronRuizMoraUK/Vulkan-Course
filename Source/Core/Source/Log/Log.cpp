#include <Log/Log.h>
#include <Debug/Debug.h>

#include <cstdio>
#include <cstdarg>
#include <cstdlib>

#pragma DX_DISABLE_WARNING(4996, "")

namespace DX::Internal
{
    void Log(LogLevel level, const char* title, const char* message, ...)
    {
        char buffer[8 * 1024];
        char* head = buffer;

        LogColor logColor = LogColor::Normal;
        head += std::sprintf(head, "[%s] ", title);

        switch(level)
        {
            case LogLevel::Verbose:
                logColor = LogColor::Blue;
                break;
            case LogLevel::Warning:
                logColor = LogColor::Yellow;
                head += std::sprintf(head, "Warning: ");
                break;
            case LogLevel::Error:
                logColor = LogColor::Red;
                head += std::sprintf(head, "Error: ");
                break;
            case LogLevel::Fatal:
                logColor = LogColor::Red;
                head += std::sprintf(head, "Fatal Error: ");
                break;
            case LogLevel::Info:
            default:
                break;
        }

        va_list args;
        va_start(args, message);
        head += vsprintf(head, message, args);
        va_end(args);

        std::sprintf(head, "\n");

        DebugOutput(logColor, buffer);

        if (level == LogLevel::Fatal)
        {
            std::abort();
        }
    }
} // namespace DX::Internal
