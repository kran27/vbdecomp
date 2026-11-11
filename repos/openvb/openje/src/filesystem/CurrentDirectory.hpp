// VC7 CRT forwarding for existing game callers.
#pragma once
#ifdef GetCurrentDirectory
#undef GetCurrentDirectory
#endif
namespace JE
{
    namespace LegacyFilesystem
    {
        char* __cdecl GetCurrentDirectory(char*, int);
        char* __cdecl GetDriveCurrentDirectory(int, char*, int);
    }
}
