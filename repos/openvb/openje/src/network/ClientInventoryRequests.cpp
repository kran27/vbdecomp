#include "network/EngineClient.hpp"
#include "network/Packet.hpp"

namespace JE
{
	//Original58A980 is JMP58A2D0; actual reliable client request.
	void Client::RequestInventoryTransfer(DWORD destinationID, DWORD itemID, DWORD sourceID)
	{
		Net::Packet request; //Actual45F300/45F280, normal real stack lifetime.
		request.field0x30 = 49;
		request.WriteDword(destinationID);
		request.WriteDword(itemID);
		request.WriteDword(sourceID);
		Send(request, 1);
	}
}
