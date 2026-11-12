// Game-owned dependency bridge; avoids an engine-header dependency on OpenVB.
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "gui/Interfaces.hpp"
#include "gui/DialogueWindow.hpp"
#include <stdexcept>

namespace JE
{
	namespace DialogueWindowServices
	{
		Client& GetClient()
		{
			// Original5924FD..59250B uses global70BD90, then member+2C. It has
			// no mode or NULL guard. The exception only names the unresolved or
			// invalid missing-owner domain; it must never provide successful output.
			return *static_cast<Client*>(F3::g_pNetworkInstance->client);
		}
		void RequestClose(Net::Packet& packet)
		{
			GetClient().SendDialogueClose(packet);
		}
		void GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height)
		{
			GetClient().child0xA0->GetScreenRectangle(x, y, width, height);
		}
	}
}
