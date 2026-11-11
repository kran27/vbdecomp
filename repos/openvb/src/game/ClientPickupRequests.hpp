#pragma once

#include <winsock2.h>
#include <windows.h>

namespace JE { namespace Net { class Client; class Packet; } }

namespace F3
{
	BYTE ReceiveGameServerPickupRequest(JE::Net::Packet& packet); //46DD70 case82; no action clearing.
}
