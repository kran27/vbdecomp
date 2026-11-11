#include "EngineServer.hpp"

namespace JE
{
	void Server::FloatingSpeechText(JE::Net::UInt32 id,JE::Net::UInt32 stringReference,JE::Net::UInt32 duration,
		JE::Net::UInt32 red,JE::Net::UInt32 green,JE::Net::UInt32 blue)
	{
		Net::Packet notification;
		notification.field0x30=47;
		notification.WriteDword(id);
		notification.WriteSignedDword(static_cast<Net::Int32>(stringReference));
		notification.WriteSignedDword(static_cast<Net::Int32>(duration));
		notification.WriteSignedDword(static_cast<Net::Int32>(red));
		notification.WriteSignedDword(static_cast<Net::Int32>(green));
		notification.WriteSignedDword(static_cast<Net::Int32>(blue));
		Broadcast(notification,1); //No extra Network/Client gate in original554D30.
	}
}
