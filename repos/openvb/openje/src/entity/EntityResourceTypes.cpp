#include "EntityResourceTypes.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <string.h>

namespace JE { namespace Common
{
	BYTE ExtractExtension(char* destination, const char* source)
	{
		if (!destination) FatalError("Common::ExtractExtension() - Destination string is NULL.");
		if (!source) FatalError("Common::ExtractExtension() - Source string is NULL.");
		const char* dot = strrchr(source, '.');
		if (!dot) return 0;
		strcpy(destination, dot);
		return 1;
	}
}}
