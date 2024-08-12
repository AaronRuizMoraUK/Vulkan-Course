#pragma once

// -------------------------------------------------------
// Usage:
//
// DX_LOG(Info, "Title", "My message %d", 123);
// DX_LOG(Verbose, "Title", "My message %d", 123);
// DX_LOG(Warning, "Title", "My message %d", 123);
// DX_LOG(Error, "Title", "My message %d", 123);
// DX_LOG(Fatal, "Title", "My message %d", 123);
// -------------------------------------------------------

#ifndef NDEBUG
// No Release configuration

#define DX_LOG(level, title, message, ...)    ::DX::Internal::Log(::DX::Internal::LogLevel::level, title, message, ##__VA_ARGS__)

#else
// Release configuration

#define DX_LOG(...)

// Disable warning C4100: unreferenced formal parameter
// This happens often in release configuration when using DX_LOG()
#include <Debug/Debug.h>
#pragma DX_DISABLE_WARNING(4100, "")

#endif // NDEBUG

namespace DX::Internal
{
    enum class LogLevel
    {
        Info,
        Verbose,
        Warning,
        Error,
        Fatal
    };

    void Log(LogLevel level, const char* title, const char* message, ...);
} // namespace DX::Internal
