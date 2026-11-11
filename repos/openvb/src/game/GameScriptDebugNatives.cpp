#include <winsock2.h>
#include "GameScriptNatives.hpp"
#include "GameScriptDebugState.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include "scripting/ScriptDebugger.hpp"
#include "gui/DebugToolsInterface.hpp"
#include "../startup/GameScriptingComponent.hpp"
#include "../startup/NetworkComponent.hpp"
#include "small_console.h"
#include <stdarg.h>
#include <stdio.h>

using F3::ScriptDebugState::debugText;
using F3::ScriptDebugState::debugLength;

//5C32F0. The reused Small formatter ignores the callback's machine return value.
extern "C" void F3_DebugPutChar(AMX* vm, int ch)
{
	DWORD next = debugLength + 1;
	if (next >= sizeof(debugText))
	{
		F3::LogScriptError(vm, "Debug() formatted string too long.");
		return;
	}
	debugText[debugLength] = static_cast<char>(ch);
	debugLength = next;
}

//5C3320. Preserve original vsprintf/error/clamp order, including the unbounded
//CRT formatting before the size check; this is not a safer replacement policy.
extern "C" void F3_DebugPrintf(AMX* vm, const char* format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	int written = vsprintf(debugText + debugLength, format, arguments);
	va_end(arguments);
	if (written >= 0)
	{
		debugLength += written;
		if (debugLength >= sizeof(debugText))
		{
			F3::LogScriptError(vm, "Debug() formatted string too long.");
			debugLength = sizeof(debugText) - 1;
		}
	}
	else
	{
		F3::LogScriptError(vm, "Debug() error formatting string.");
		debugText[0] = 0;
		debugLength = 0;
	}
}

namespace F3
{
	cell AMX_NATIVE_CALL NativeDebug(AMX* vm, cell* parameters) //5C3710; actual derived DebugWindow writer.
	{
		cell* originalParameters = parameters;
		debugText[0] = 0;
		debugLength = 0;
		// Original reuses the parameter home as GetAddr output and ignores failure.
		amx_GetAddr(vm, originalParameters[1], &parameters);
		F3_SmallPrintString(vm, parameters, originalParameters + 2,
			static_cast<int>((static_cast<DWORD>(originalParameters[0]) >> 2) - 1));
		debugText[debugLength] = 0;
		// Original passes the buffer as the format, without a "%s" wrapper.
		g_pScriptingInstance->DebugOutput(debugText);
		return 1;
	}

	cell AMX_NATIVE_CALL NativeBreakpoint(AMX* vm, cell* parameters) //5C33A0; actual controls and debugger display.
	{
		if (!VerifyScriptParameters(vm, parameters, 0, "Breakpoint")) return 0;
		JE::Server* server = static_cast<JE::Server*>(g_pNetworkInstance->server);
		if (!server->field0x70EC) return 0;
		DWORD selectedOwner = server->field0x70E8;
		void* owner = NULL;
		amx_GetUserData(vm, 0x464C534D, &owner); // MYSL; original ignores the API result.
		if (selectedOwner != reinterpret_cast<DWORD>(owner)) return 0;
		void* state; // Original leaves this output unwritten if STAT lookup fails.
		amx_GetUserData(vm, 0x54415453, &state);
		if (state != reinterpret_cast<void*>(1)) return 0;
		amx_SetUserData(vm, 0x454E494C, reinterpret_cast<void*>(vm->curline)); // LINE.
		amx_RaiseError(vm, AMX_ERR_SLEEP);
		state = reinterpret_cast<void*>(2);
		amx_SetUserData(vm, 0x54415453, state);
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x94->EnableControls(1);
		JE::ScriptDebug::DisplayState(vm, 1, 1);
		return 1;
	}
}
