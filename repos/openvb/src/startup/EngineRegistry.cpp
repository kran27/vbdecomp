#include "EngineRegistry.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace F3
{
	// 0x5ACD60. Capture each live factory, then its returned object.
	void Startup()
	{
		JE::LogDebugString("Startup()\r\n");
		for (std::size_t nIndex = 0; nIndex < COMPONENT_COUNT; ++nIndex)
		{
			ComponentFactory pFactory = g_Components[nIndex].pFactory;
			if (pFactory)
			{
				GameComponent* pObject = pFactory();
				g_Components[nIndex].pObject = pObject;
				pObject->Initialize();
			}
		}
	}

	// 0x5ACDA0. Cleanup may replace/clear the slot; delete its reloaded value.
	void Shutdown()
	{
		JE::LogDebugString("Shutdown()\r\n");
		for (std::size_t nIndex = COMPONENT_COUNT; nIndex-- > 0;)
		{
			g_Components[nIndex].pObject->Cleanup();
			GameComponent* pObject = g_Components[nIndex].pObject;
			if (pObject)
				delete pObject;
			g_Components[nIndex].pObject = NULL;
		}
	}

	// 0x5ACDE0. Every live slot is called in order, without a null shortcut.
	void UpdateComponents()
	{
		for (std::size_t nIndex = 0; nIndex < COMPONENT_COUNT; ++nIndex)
			g_Components[nIndex].pObject->Update();
	}
}
