#include "GameScriptNatives.hpp"

namespace F3
{
	cell AMX_NATIVE_CALL NativePlayMusic(AMX* vm, cell* parameters) //45B9C0.
	{
		if (VerifyScriptParameters(vm, parameters, 1, "PlayMusic"))
			LogScriptError(vm, "PlayMusic() unimplemented.");
		return 0; //This is the original prototype's deliberate unimplemented body.
	}
}
