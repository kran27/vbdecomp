#include "EngineMapName.hpp"
#include <cstring>

void Gfx_SetEngineString248(std::string& target,const char* value) //5C0C00.
{
	//The original scans the input before calling the real string assignment.
	//No NULL fallback, filename rewrite, map lookup or renderer child is added.
	const std::string::size_type length=std::strlen(value);
	target.assign(value,length);
}
