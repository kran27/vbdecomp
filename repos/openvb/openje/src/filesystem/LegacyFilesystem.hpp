// OpenJE
#pragma once

#include <io.h>

namespace JE
{
    namespace LegacyFilesystem
    {
        // Supplied VC7 file enumeration APIs and find-data type.
        typedef _finddata_t FindData32;
        ::intptr_t __cdecl FindFirst(const char*, FindData32*);
        int __cdecl FindNext(::intptr_t, FindData32*);
        int __cdecl FindClose(::intptr_t);
        int __cdecl MakeDirectory(const char*);
    }
}
