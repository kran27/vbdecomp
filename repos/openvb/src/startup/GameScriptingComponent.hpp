#pragma once

#include "EngineRegistry.hpp"

#include <cstddef>
#include <string>

namespace JE { class DebugWindow; }

namespace F3
{
	class ScriptingComponent : public GameComponent
	{
	public:
		ScriptingComponent(); // 0x59C040
		~ScriptingComponent(); // 0x59BFB0

		void Initialize(); // 0x5B1510, dispatch to hook +20
		void Cleanup(); // 0x59C030, dispatch to hook +24
		void Update(); // 0x404B70, original slot +12
		virtual void DebugOutput(const char* pszFormat, ...); // 0x59BFD0, slot +16
		virtual void OnInitialize(); // 0x404B70, original slot +20
		virtual void OnCleanup(); // 0x404B70, original slot +24


	};

	class GameScriptingComponent : public ScriptingComponent
	{
	public:
		~GameScriptingComponent(); // 0x45E100
		void DebugOutput(const char* pszFormat, ...); // 0x46CD60
	};

	// One owner for original singleton 0x70C65C, independent of Small/AMX state.
	extern ScriptingComponent* g_pScriptingInstance;
	GameComponent* __cdecl CreateGameScripting(); // 0x45E1C0, actual registry factory.

	namespace ScriptingServices
	{
		void* Allocate(std::size_t nBytes);
		void Free(void* pBlock);

		// Historical controlled-provider fixture retains its own opaque boundary.
		typedef JE::DebugWindow DebugWindow;
		DebugWindow* GetDebugWindow();
		void WriteToDebugWindow(DebugWindow* pWindow, const std::string& sText,
			int nRed, int nGreen, int nBlue);
	}
}
