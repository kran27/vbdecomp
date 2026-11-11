#pragma once

#include <windows.h>
#include <vector>
#include <map>
#include <string>
#include "amx.h"

namespace JE
{
	namespace ScriptServices
	{
		// Explicit engine services, resolved by the actual Client/World owners.
		void* GetDebugWindow(); // Load captures Client+8C before Unload.
		BYTE ScriptSuppressed(DWORD ownerID); // EngineServer5529E0.
		BYTE ScriptsEnabled(); // Actual EngineServer+70E4 gate in Entity54C6D7.
		void DebugOutput(void* window, const char* message, BYTE red, BYTE green, BYTE blue);
		AMX_NATIVE_INFO* NativeTable(DWORD ordinal); // 7004C0,6FFAF8,6F3750 order.
	}

	// Original RTTI6D6E84 records IScript as the zero-offset base. Its
	// owner/string members end at24; the derived flags begin there.
	struct __declspec(novtable) IScript
	{
		explicit IScript(DWORD owner) { ownerID = owner; }
		virtual ~IScript(); //5A48E0, actual filename member destruction.
		virtual BYTE Load(const char* name, BYTE debug) = 0;
		virtual void Unload() = 0;
		virtual void PreLoad(const std::map<std::string,cell>& variables) = 0;
		virtual BYTE HasPublic(const char* name) = 0;
		virtual BYTE Run(const char* name, cell* result) = 0;
		virtual BYTE Run(const char* name, cell* result, int count, cell* arguments) = 0;
		virtual BYTE RunArray(const char* name, cell* result, cell* values, int count) = 0;
		virtual BYTE StepOver() = 0;
		virtual BYTE StepInto() = 0;
		virtual BYTE ContinueDebugging() = 0;
		virtual void ShowDebugState() = 0;
		DWORD ownerID; // Original +04.
		std::string filename; // Original +08, actual VC7 string.
	};

	struct Script_Small : public IScript
	{
	public:
		explicit Script_Small(DWORD owner); // 0x5A7240.
		virtual ~Script_Small(); // 0x5A75F0.
		virtual BYTE Load(const char* name, BYTE debug); // 0x5A8880.
		virtual void Unload(); // 0x5A72F0.
		virtual void PreLoad(const std::map<std::string,cell>& variables); // 0x5A4A80.
		virtual BYTE HasPublic(const char* name); // 0x5A4390.
		// VC7 emits an overload group in reverse declaration order: slots5/6.
		virtual BYTE Run(const char* name, cell* result); // 0x5A58C0.
		virtual BYTE Run(const char* name, cell* result, int count, cell* arguments); // 0x5A5A90.
		virtual BYTE RunArray(const char* name, cell* result, cell* values, int count); //5A43C0.
		virtual BYTE StepOver(); // 0x5A4450; STAT2 ->4.
		virtual BYTE StepInto(); // 0x5A44B0; STAT2 ->3.
		virtual BYTE ContinueDebugging(); // 0x5A5600; STAT2 ->1, real VSTK map clear.
		virtual void ShowDebugState(); // 0x5A73C0; actual paused argument and display5A6520.
		AMX& GetVM() { return vm; }
		BYTE debugging, fileDebug; // Original +24/+25.

	private:
		AMX vm; // Original +26, packed official runtime type, size98.
	public:
		std::vector<std::string> functions; // Original +C0, borrowed via FQUE.
	private:
		void* program; // Original +D0, malloc/free.
	public:
		std::map<LONG,std::string> files; // Original +D4, string-valued nodes (nil+2D).
	private:
		static int AMXAPI FileHook(AMX* amx); // 0x5A5860.
		static int AMXAPI Debugger(AMX* amx); //5A8030; actual data, source and UI owners.
		Script_Small(const Script_Small&);
		Script_Small& operator=(const Script_Small&);
	};
}
