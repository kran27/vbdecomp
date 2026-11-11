#include "CurrentDirectory.hpp"
#include <direct.h>
namespace JE
{
    namespace LegacyFilesystem
    {
        char* __cdecl GetCurrentDirectory(char* output, int length)
        {
            return ::_getcwd(output, length);
        }
        char* __cdecl GetDriveCurrentDirectory(int drive, char* output, int length)
        {
            return ::_getdcwd(drive, output, length);
        }
    }
}
