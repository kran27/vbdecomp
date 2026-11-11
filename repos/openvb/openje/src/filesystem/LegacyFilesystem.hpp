// OpenJE
#pragma once

#include <io.h>

namespace JE
{
    namespace LegacyFilesystem
    {
        // Compatibility names for existing game callers and comparison fixtures.
        // This is the compiler's real find-data type, not a recreated CRT object.
        typedef _finddata_t FindData32;
        ::intptr_t __cdecl FindFirst(const char*, FindData32*);
        int __cdecl FindNext(::intptr_t, FindData32*);
        int __cdecl FindClose(::intptr_t);
        int __cdecl MakeDirectory(const char*);
    }
}
