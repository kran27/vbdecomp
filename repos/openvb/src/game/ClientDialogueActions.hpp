#pragma once

#include <winsock2.h>
#include <windows.h>

namespace JE
{
	class Server;
	namespace Net { class Client; class Packet; }
}

namespace F3
{
	BYTE ReceiveGameServerDialogueRequest(JE::Server& server, JE::Net::Packet& packet); //46DD70 case73.
}
