#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "gui/WorldContainerServices.hpp"
#include "gui/DebugWindow.hpp"
#include <stdexcept>

namespace JE
{
	namespace
	{
		WorldContainerExternalServices* externalServices = NULL;
		WorldContainerExternalServices& External()
		{
			if (!externalServices)
				throw std::logic_error("WorldContainer457DB0: actual CSound/debug child remains unresolved");
			return *externalServices;
		}
	}
	WorldContainerExternalServices* SetWorldContainerExternalServices(WorldContainerExternalServices* services)
	{
		WorldContainerExternalServices* previous = externalServices;
		externalServices = services;
		return previous;
	}
	namespace DialogueWindowServices
	{
		void GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height);
	}
	namespace WorldContainerServices
	{
		void InitializeClickSound() { External().InitializeClickSound(); }
		void LoadClickSound(const char* name, BYTE value) { External().LoadClickSound(name, value); }
		void PlayClickSound(DWORD first, DWORD second, DWORD third) { External().PlayClickSound(first, second, third); }
		void DestroyClickSound() { External().DestroyClickSound(); }
		DebugWindow* CaptureInspectionDebug()
		{
			if (externalServices) return NULL;
			return static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x8C;
		}
		void WriteInspectionDebug(DebugWindow* window)
		{
			if (externalServices)
			{
				externalServices->WriteInspectionDebug();
				return;
			}
			window->Write(std::string("Inspecting entity."),128,128,128);
		}
		Client& GetClient()
		{
			if (!F3::g_pNetworkInstance || !F3::g_pNetworkInstance->client)
				throw std::logic_error("WorldContainer4575D0: actual published Network/client required");
			return *static_cast<Client*>(F3::g_pNetworkInstance->client);
		}
		void GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height)
		{
			//Both originals use the same Client+A0 virtual20 rectangle. Reuse
			//its explicit binding; no second HUD/screen state is introduced.
			DialogueWindowServices::GetScreenRectangle(x, y, width, height);
		}
	}
}
