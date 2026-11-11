#include <winsock2.h>
#include "GameScriptingComponent.hpp"
#include "CommonException.hpp"

#include "diagnostics/DebugOutput.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "gui/DebugWindow.hpp"
#include "scripting/ScriptSmall.hpp"

#include <cstdarg>
#include <cstdio>
#include <new>

namespace F3
{
	ScriptingComponent* g_pScriptingInstance = NULL;
	static const char g_szDuplicateScripting[] = "Scripting object already exists.";

	ScriptingComponent::ScriptingComponent()
	{
		// C++ installs the base vptr before this body. A failed base constructor
		// does not run this destructor and must not clear the existing singleton.
		if (g_pScriptingInstance)
			throw Common::Exception(g_szDuplicateScripting);
		g_pScriptingInstance = this;
	}

	ScriptingComponent::~ScriptingComponent()
	{
		// Original 0x59BFB0 clears it even if it points at a different object.
		g_pScriptingInstance = NULL;
	}

	GameScriptingComponent::~GameScriptingComponent()
	{
		// C++ performs derived-to-base destruction, with no extra game cleanup.
	}

	void ScriptingComponent::Initialize()
	{
		OnInitialize();
	}

	void ScriptingComponent::Cleanup()
	{
		OnCleanup();
	}

	void ScriptingComponent::Update()
	{
		// Original 0x404B70 has no behavior or meaningful C++ return value.
	}

	void ScriptingComponent::OnInitialize()
	{
		// Actual base and game tables both install 0x404B70 here.
	}

	void ScriptingComponent::OnCleanup()
	{
		// Actual base and game tables both install 0x404B70 here.
	}

	void ScriptingComponent::DebugOutput(const char* pszFormat, ...)
	{
		char szBuffer[1024];
		va_list args;
		va_start(args, pszFormat);
		std::vsprintf(szBuffer, pszFormat, args);
		va_end(args);

		// Deliberate second format pass, exactly as at original 0x59C008.
		JE::LogDebugString(szBuffer);
	}

	void GameScriptingComponent::DebugOutput(const char* pszFormat, ...)
	{
		char szBuffer[1024];
		va_list args;
		va_start(args, pszFormat);
		std::vsprintf(szBuffer, pszFormat, args);
		va_end(args);

		// Original: [0x70BD90] -> member +0x2C -> member +0x8C.
		// RTTI identifies 0x70BD90 as the Network singleton; the old HudBase name
		// is misleading. The first two pointers have no original null guard.
		// Capture the current Client after formatting, then its optional window.
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		JE::DebugWindow* pWindow = client->child0x8C;
		if (pWindow)
		{
			// The real owning string is released before the final logger call.
			// It ends before the final logger call, including callback exceptions.
			std::string sText(szBuffer);
			pWindow->Write(sText, 128, 128, 128);
		}
		JE::LogDebugString("%s\n", szBuffer);
	}

	GameComponent* __cdecl CreateGameScripting()
	{
		return new GameScriptingComponent;
	}


	namespace ScriptingServices
	{
		void* Allocate(std::size_t nBytes)
		{
			return ::operator new(nBytes);
		}

		void Free(void* pBlock)
		{
			::operator delete(pBlock);
		}

		DebugWindow* GetDebugWindow()
		{
			//46CDAA..46CDB2 is the same Network+2C -> Client+8C chain used by
			//ScriptSmall. The default service reads the published Client; an explicit
			//environment retains precedence. A NULL final window remains optional here.
			return static_cast<DebugWindow*>(JE::ScriptServices::GetDebugWindow());
		}

		void WriteToDebugWindow(DebugWindow* window, const std::string& text, int red, int green, int blue)
		{
			window->Write(text, red, green, blue); //Actual proprietary owner5583E0.
		}
	}
}
