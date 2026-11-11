#pragma once

#include <windows.h>
#include "amx.h"
#include <d3dx8math.h>

namespace JE
{
	// Explicit host boundary for Client debug-window and EngineServer script
	// gates. This is neither an original owner nor a second server state.
	class ScriptEnvironment
	{
	public:
		virtual ~ScriptEnvironment() {}
		virtual void* GetDebugWindow() = 0;
		virtual BYTE ScriptSuppressed(DWORD ownerID) = 0;
		virtual BYTE ScriptsEnabled() = 0;
		virtual AMX_NATIVE_INFO* NativeTable(DWORD ordinal) = 0;
		virtual void DebugOutput(void* window, const char* message, BYTE red, BYTE green, BYTE blue) = 0;
	};
	extern ScriptEnvironment* g_ScriptEnvironment;
	namespace ScriptServices
	{
		BYTE ReadNativeFloat(AMX* vm, cell value, float& output); //59AA50, raw cell bits; does not resolve an address.
		BYTE WriteNativeFloat(AMX* vm, cell address, float value); //59AA70, actual Small address lookup then float store.
		BYTE WriteNativeInteger(AMX* vm, cell address, cell value); //59AAA0.
		BYTE ReadNativeString(AMX* vm,cell address,char* output,DWORD capacity); //59AB20, real Small string APIs.
		BYTE WriteNativeString(AMX* vm,cell address,const char* value); //59AB80, actual address lookup/unpacked output.
		BYTE ReadNativeVector(AMX* vm, cell address, D3DXVECTOR3& output); //59AC40/59ABC0, copies3 cells.
	}
}
