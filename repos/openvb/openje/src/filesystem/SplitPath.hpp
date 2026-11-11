// VC7 CRT forwarding for existing game callers.
#pragma once
namespace JE
{
    namespace LegacyFilesystem
    {
        void __cdecl SplitPath(const char*, char*, char*, char*, char*);
    }
}
