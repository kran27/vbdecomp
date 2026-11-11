// Game-owned dependency bridge; avoids an engine-header dependency on OpenVB.
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "gui/Interfaces.hpp"
#include "gui/DialogueWindow.hpp"
#include "gui/DialogueServices.hpp"
#include <stdexcept>

namespace JE
{
	namespace
	{
		DialogueScreenServices* screenServices = NULL;
	}
	DialogueScreenServices* SetDialogueScreenServices(DialogueScreenServices* services)
	{
		DialogueScreenServices* previous = screenServices;
		screenServices = services;
		return previous;
	}
	namespace DialogueWindowServices
	{
		Client& GetClient()
		{
			// Original5924FD..59250B uses global70BD90, then member+2C. It has
			// no mode or NULL guard. The exception only names the unresolved or
			// invalid missing-owner domain; it must never provide successful output.
			if (!F3::g_pNetworkInstance || !F3::g_pNetworkInstance->client)
				throw std::logic_error("Dialogue5924FD: actual Network/client owner required");
			return *static_cast<Client*>(F3::g_pNetworkInstance->client);
		}
		void RequestClose(Net::Packet& packet)
		{
			GetClient().SendDialogueClose(packet);
		}
		void GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height)
		{
			Client& client = GetClient();
			if (client.child0xA0)
			{
				client.child0xA0->GetScreenRectangle(x, y, width, height);
				return;
			}
			if (!screenServices)
				throw std::logic_error("Dialogue5921B0: actual Client+A0 screen rectangle owner unresolved");
			screenServices->GetScreenRectangle(x, y, width, height);
		}
	}
}
