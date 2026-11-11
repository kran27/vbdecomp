#pragma once

#include <winsock2.h>
#include <windows.h>

namespace JE { namespace Net { class Packet; } }

namespace F3
{
	class GameServer;
	BYTE ReceiveGameServerWeatherRequest(GameServer& server, JE::Net::Packet& packet);
}
