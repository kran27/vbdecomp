#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "gui/WorldContainerServices.hpp"
#include "gui/DebugWindow.hpp"
#include <stdexcept>

namespace JE
{
	namespace DialogueWindowServices
	{
		void GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height);
	}
	namespace WorldContainerServices
	{
		DebugWindow* CaptureInspectionDebug()
		{
			return static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x8C;
		}
		void WriteInspectionDebug(DebugWindow* window)
		{
			window->Write(std::string("Inspecting entity."),128,128,128);
		}
		Client& GetClient()
		{
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
