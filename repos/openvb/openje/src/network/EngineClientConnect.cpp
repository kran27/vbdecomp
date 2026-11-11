#include "EngineClient.hpp"
#include "startup/NetworkComponent.hpp"

namespace JE
{
	void Client::Connect(const char* host) //45E6B0.
	{
		Net::Client::Connect(host);
	}

	void Client::Connect(const Net::Address& address) //587770.
	{
		//Original writes the caller's port. The reconstructed Net signature is
		//const-qualified; this local adaptation preserves the observed mutation.
		if (!address.port) const_cast<Net::Address&>(address).port = 21498;
		DWORD mode = 2;
		Net::Address local(127, 0, 0, 1, F3::g_pNetworkInstance->serverPort);
		if (static_cast<Net::UInt8>(address.Compare(local, 1))) mode = 3;
		F3::g_pNetworkInstance->SetMode(mode);
		Net::Client::Connect(address);
	}
}
