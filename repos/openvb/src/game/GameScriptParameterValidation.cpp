#include "GameScriptNatives.hpp"
#include "entity/Entity.hpp"

namespace F3
{
	BYTE VerifyScriptParameters(AMX* vm, cell* parameters, DWORD count, const char* name)
	{
		DWORD provided = static_cast<DWORD>(parameters[0]) >> 2;
		if (provided != count)
		{
			LogScriptError(vm, "Invalid number of paramters passed to function %s().", name);
			LogScriptError(vm, "Expected %d parameters but recieved %d.", count, provided);
			return 0;
		}
		return 1;
	}

	BYTE ResolveScriptEntity(AMX* vm, DWORD id, JE::Entity** output, BYTE requireActive)
	{
		*output = JE::GetEntityPtr(id);
		if (!*output) return 0;
		if (requireActive && !(*output)->renderEnabled)
			LogScriptError(vm, "Attempt to use inactive entity in script.");
		// Actual59AF2E always returns1, even following the inactive diagnostic.
		return 1;
	}

	BYTE VerifyMinimumScriptParameters(AMX* vm, cell* parameters, LONG minimum,
		const char* name, LONG* provided)
	{
		*provided = static_cast<DWORD>(parameters[0]) >> 2;
		if (*provided < minimum)
		{
			LogScriptError(vm, "Invalid number of paramters passed to function %s().", name);
			LogScriptError(vm, "Expected at least %d parameters but recieved %d.", minimum, *provided);
			return 0;
		}
		return 1;
	}
}
