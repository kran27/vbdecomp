#include "Server.hpp"
#include "Connection.hpp"
#include <cstdlib>

namespace JE { namespace Net
{
	void Server::ReceiveDiscoveryRequest(Packet& request) //5A0C20.
	{
		if (!field0x59) return;
		char source[64];
		request.field0x0C = 0;
		debugStream.Print(0x2000,"[server] Receiving a request for info from %s\n",
			request.address.Format(source,sizeof(source),1));
		if (!OnDiscoveryRequest(request)) return;
		Packet reply;
		reply.address = request.address;
		reply.field0x30 = 2;
		debugStream.Print(4,"[server] Sending requested info to %s\n",
			request.address.Format(source,sizeof(source),1));
		//Original5A0CF3/5A0CF9 pass the request, not the newly constructed reply.
		PrepareDiscoveryResponse(request);
		SendOutOfBand(request);
	}

	void Server::ReceiveChallengeRequest(Packet& request) //5A0D60.
	{
		char source[64];
		request.field0x0C = 0;
		debugStream.Print(0x4000,"[server] Receiving a challenge request from %s\n",
			request.address.Format(source,sizeof(source),1));
		if (!OnChallengeRequest(request)) return;
		UInt32 oldestIndex = 0;
		UInt32 oldestTime = 0x7FFFFFFF;
		UInt32 index = 0;
		do
		{
			if (UInt8(slots[index].address.Compare(request.address,0))) break;
			if (slots[index].field0x10 < oldestTime)
			{
				oldestTime = slots[index].field0x10;
				oldestIndex = index;
			}
			++index;
		}
		while (index < 1024);
		if (index == 1024)
		{
			slots[oldestIndex].address = request.address;
			slots[oldestIndex].field0x0C = std::rand() & 0x7FFF;
			UInt32 currentTime = field0x34;
			slots[oldestIndex].field0x10 = currentTime;
			index = oldestIndex;
		}
		//A cache hit does not renew its time or regenerate the challenge.
		Packet reply;
		reply.address = request.address;
		reply.field0x30 = 3;
		reply.WriteDword(slots[index].field0x0C);
		debugStream.Print(8,"[server] Sending requested challenge to client at %s\n",
			request.address.Format(source,sizeof(source),1));
		PrepareChallengeResponse(reply);
		SendOutOfBand(reply);
	}

	void Server::ReceiveConnectRequest(Packet& request) //5A0F30.
	{
		char source[64];
		request.field0x0C = 0;
		Packet reply;
		reply.address = request.address;
		reply.field0x30 = 4;
		UInt32 count = connectionCount;
		UInt32 index = 0;
		for (; index < connectionCount; ++index)
			if (!connections[index]->state) break;
		if (index >= count)
		{
			reply.WriteDword(1);
			debugStream.Print(16,"[server] Denying connection from client at %s. Server full.\n",
				request.address.Format(source,sizeof(source),1));
		}
		else if (request.field0x10 - request.field0x0C < 4)
		{
			reply.WriteDword(1);
			debugStream.Print(16,"[server] Denying connection from client at %s. Malformed packet.\n",
				request.address.Format(source,sizeof(source),1));
		}
		else
		{
			UInt32 available = request.field0x10;
			UInt32 protocol;
			request.ReadDword(protocol);
			if (protocol != 1)
			{
				reply.WriteDword(1);
				debugStream.Print(16,"[server] Denying connection from client at %s. Invalid protocol.\n",
					request.address.Format(source,sizeof(source),1));
				PrepareConnectResponse(reply);
				SendOutOfBand(reply);
				return;
			}
			else if (available - request.field0x0C < 4)
			{
				reply.WriteDword(1);
				debugStream.Print(16,"[server] Denying connection from client at %s. Malformed packet.\n",
					request.address.Format(source,sizeof(source),1));
				PrepareConnectResponse(reply);
				SendOutOfBand(reply);
				return;
			}
			else
			{
				UInt32 challenge;
				request.ReadDword(challenge);
				UInt32 slot = 0;
				for (; slot < 1024; ++slot)
					if (UInt8(slots[slot].address.Compare(request.address,0))
						&& slots[slot].field0x0C == challenge) break;
				if (slot >= 1024)
				{
					reply.WriteDword(1);
					debugStream.Print(16,"[server] Denying connection from client at %s. Invalid challenge.\n",
						request.address.Format(source,sizeof(source),1));
				}
				else
				{
					debugStream.Print(0x8000,"[server] Receiving a connect request from %s\n",
						request.address.Format(source,sizeof(source),1));
					if (!OnConnectRequest(request))
					{
						reply.WriteDword(1);
						debugStream.Print(16,"[server] Denying connection from client at %s\n",
							request.address.Format(source,sizeof(source),1));
					}
					else
					{
						connections[index]->Open(&socket,field0x2C,request.address,request.field0x24,field0x44);
						static_cast<ServerClientConnection*>(connections[index])->field0x4164 = 0;
						++field0x60;
						reply.WriteDword(0);
						debugStream.Print(16,"[server] Accepting connection from client at %s\n",
							request.address.Format(source,sizeof(source),1));
					}
				}
			}
		}
		PrepareConnectResponse(reply);
		SendOutOfBand(reply);
	}
} }
