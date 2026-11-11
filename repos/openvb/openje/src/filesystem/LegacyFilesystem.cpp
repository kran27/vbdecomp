// Forward to the actual supplied VC7 CRT; no CRT implementation is reconstructed.
#include "LegacyFilesystem.hpp"
#include <direct.h>

namespace JE
{
    namespace LegacyFilesystem
    {
        ::intptr_t __cdecl FindFirst(const char* path, FindData32* data)
        {
            return ::_findfirst(path, data);
        }
        int __cdecl FindNext(::intptr_t handle, FindData32* data)
        {
            return ::_findnext(handle, data);
        }
        int __cdecl FindClose(::intptr_t handle)
        {
            return ::_findclose(handle);
        }
        int __cdecl MakeDirectory(const char* path)
        {
            return ::_mkdir(path);
        }
    }
}
