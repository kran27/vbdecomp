#include "EngineClient.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE
{
	void Client::ReadClientIdentity(Net::Packet& reply)
	{
		//58A657..58A683: same wire DWORD written by Server552CA0. ReadDword
		//publishes directly into the canonical field; underflow leaves it intact.
		reply.ReadDword(clientID);
		LogDebugString("Client: connected to server.  My id is %u\n",clientID);
		field0x108=0;
		field0x10A=0;
		//The complete callback next dereferences Graphics+56, optionally plays
		//startup movies, and in client-only mode reads World state/sends67.
		//This explicit prefix neither enters those owners nor reports greeting
		//success or changes Net::Client.state.
	}
}
