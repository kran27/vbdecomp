#pragma once

#include <windows.h>
#include <cstdio>

// DXErr8 is an obsolete static library. Retain only its diagnostic call sites.
inline const char* DXGetErrorString8A(HRESULT result)
{
    static thread_local char text[32];
    std::snprintf(text, sizeof(text), "HRESULT 0x%08lX", static_cast<unsigned long>(result));
    return text;
}

inline const char* DXGetErrorDescription8A(HRESULT result)
{
    static thread_local char text[512];
    const DWORD count = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                                       static_cast<DWORD>(result), 0, text, static_cast<DWORD>(sizeof(text)), nullptr);
    if (count == 0)
    {
        std::snprintf(text, sizeof(text), "HRESULT 0x%08lX", static_cast<unsigned long>(result));
    }
    return text;
}
