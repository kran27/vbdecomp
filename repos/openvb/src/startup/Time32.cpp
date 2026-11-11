#include "Time32.hpp"
namespace F3
{

	Time32Value __cdecl Time32(Time32Value* output)
	{
		return ::time(output);
	}
}
