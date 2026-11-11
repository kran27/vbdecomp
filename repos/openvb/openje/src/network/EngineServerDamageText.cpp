#include "EngineServer.hpp"
#include <cstring>

namespace JE
{
	void Server::FloatingDamageText(JE::Net::UInt32 id, const char* text,
		JE::Net::UInt32 red, JE::Net::UInt32 green, JE::Net::UInt32 blue) //555F10 ->554E50.
	{
		Net::Packet notification;
		notification.field0x30 = 48;
		notification.WriteDword(id);
		notification.Write(text, JE::Net::UInt32(std::strlen(text) + 1));
		notification.WriteSignedDword(static_cast<Net::Int32>(red));
		notification.WriteSignedDword(static_cast<Net::Int32>(green));
		notification.WriteSignedDword(static_cast<Net::Int32>(blue));
		Broadcast(notification,1);
	}
}
