#include "GameScriptNatives.hpp"
#include "scripting/ScriptEnvironment.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativeSample(AMX* vm, cell* parameters) //5CA820.
	{
		float value;
		char text[128];
		if (!VerifyScriptParameters(vm, parameters, 3, "Sample")) return 0;
		if (!JE::ScriptServices::ReadNativeFloat(vm, parameters[2], value))
		{
			LogScriptError(vm, "Sample() was unable to retrieve second paramter.");
			return 0;
		}
		if (!JE::ScriptServices::ReadNativeString(vm, parameters[3], text, sizeof(text)))
		{
			LogScriptError(vm, "Sample() was unable to retrieve third paramter.");
			return 0;
		}
		if (!JE::ScriptServices::WriteNativeInteger(vm, parameters[1], 25))
		{
			LogScriptError(vm, "Sample() was unable to set the reference integer.");
			return 0;
		}
		value = 3.3f;
		if (!JE::ScriptServices::WriteNativeFloat(vm, parameters[2], value))
		{
			LogScriptError(vm, "Sample() was unable to set the reference float.");
			return 0;
		}
		return 1; //5CA90F; the original prototype's inferred void type is wrong.
	}
}
