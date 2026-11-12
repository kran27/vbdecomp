#include <winsock2.h>
#include "ScriptEnvironment.hpp"
#include "ScriptSmall.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include "gui/DebugWindow.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/GameScriptNatives.hpp"
#include "game/GameScriptMapContext.hpp"
#include <string.h>

namespace JE
{
	namespace
	{
		//Source integration only; no second Client, Server, VM or native table.
		class NativeScriptEnvironment : public ScriptEnvironment
		{
		public:
			virtual void* GetDebugWindow()
			{
				return static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x8C;
			}
			virtual BYTE ScriptSuppressed(DWORD ownerID)
			{
				return static_cast<Server*>(F3::g_pNetworkInstance->server)->ScriptSuppressed(ownerID);
			}
			virtual BYTE ScriptsEnabled()
			{
				return static_cast<Server*>(F3::g_pNetworkInstance->server)->field0x70E4;
			}
			virtual AMX_NATIVE_INFO* NativeTable(DWORD ordinal) { return F3::GetScriptNativeTable(ordinal); }
			virtual void DebugOutput(void* window, const char* message, BYTE red, BYTE green, BYTE blue)
			{
				static_cast<DebugWindow*>(window)->Write(std::string(message), red, green, blue);
			}
		};
		ScriptEnvironment& Environment()
		{
			static NativeScriptEnvironment native;
			return native;
		}
	}
	namespace ScriptServices
	{
	BYTE ReadNativeStringLength(AMX* vm, cell address, int& length) //59AAD0.
	{
		length = 0;
		cell* source;
		int measuredLength;
		if (amx_GetAddr(vm, address, &source) != 0 || amx_StrLen(source, &measuredLength) != 0) return 0;
		length = measuredLength;
		return 1;
	}

		BYTE ReadNativeFloat(AMX*, cell value, float& output) //59AA50.
		{
			memcpy(&output, &value, sizeof(cell));
			return 1;
		}
		BYTE WriteNativeFloat(AMX* vm, cell address, float value) //59AA70.
		{
			cell* target;
			if (amx_GetAddr(vm, address, &target)) return 0;
			*reinterpret_cast<float*>(target) = value;
			return 1;
		}
		BYTE WriteNativeInteger(AMX* vm, cell address, cell value) //59AAA0.
		{
			cell* target;
			if (amx_GetAddr(vm, address, &target)) return 0;
			*target = value;
			return 1;
		}
		BYTE ReadNativeVector(AMX* vm, cell address, D3DXVECTOR3& output)
		{
			cell values[3];
			if (!F3::ReadScriptCells(vm, address, values, 3)) return 0;
			ReadNativeFloat(vm, values[0], output.x);
			ReadNativeFloat(vm, values[1], output.y);
			ReadNativeFloat(vm, values[2], output.z);
			return 1;
		}
		BYTE ReadNativeString(AMX* vm,cell address,char* output,DWORD capacity)
		{
			cell* source;
			int length;
			if (amx_GetAddr(vm, address, &source)) return 0;
			if (amx_StrLen(source, &length)) return 0;
			if (static_cast<DWORD>(length) >= capacity) return 0;
			return amx_GetString(output, source) == 0;
		}
		BYTE WriteNativeString(AMX* vm,cell address,const char* value) //59AB80.
		{
			cell* target;
			if (amx_GetAddr(vm,address,&target)) return 0;
			amx_SetString(target,const_cast<char*>(value),0);
			return 1;
		}
		void* GetDebugWindow() { return Environment().GetDebugWindow(); }
		BYTE ScriptSuppressed(DWORD ownerID) { return Environment().ScriptSuppressed(ownerID); }
		BYTE ScriptsEnabled() { return Environment().ScriptsEnabled(); }
		AMX_NATIVE_INFO* NativeTable(DWORD ordinal) { return Environment().NativeTable(ordinal); }
		void DebugOutput(void* window, const char* message, BYTE red, BYTE green, BYTE blue)
		{
			Environment().DebugOutput(window, message, red, green, blue);
		}
	}
}
