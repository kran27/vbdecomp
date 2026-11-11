#pragma once

#include <winsock2.h>
#include <windows.h>
#include <d3dx8.h>

namespace JE
{
	namespace Net { class Client; class Packet; }
	// Explicit factors of the actual Client builders589FD0/58A0E0 and
	// Server556EA0 cases59/60. They do not initialize a complete peer graph.
	void SendClientWorldAction(Net::Client& client, DWORD message, DWORD entityID,
		const D3DXVECTOR3& position);
	BYTE ReceiveServerWorldAction(Net::Packet& packet);
}
