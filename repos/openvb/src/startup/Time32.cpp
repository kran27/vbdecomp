#include "Time32.hpp"
namespace F3
{
	// Use the real compiler runtime. The old FILETIME implementation is retired.
	Time32Value __cdecl Time32(Time32Value* output)
	{
		return ::time(output);
	}
}
