#include "SplitPath.hpp"
#include <cstdlib>
namespace JE
{
    namespace LegacyFilesystem
    {
        void __cdecl SplitPath(const char* source, char* drive, char* directory,
            char* filename, char* extension)
        {
            ::_splitpath(source, drive, directory, filename, extension);
        }
    }
}
