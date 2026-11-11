// Original WinSock import-library thunk linkage.
#define WINSOCK_API_LINKAGE
#include "Socket.hpp"
#include "Packet.hpp"
#include <cstring>

namespace JE
{
	namespace Net
	{
		void Socket::Send(const Packet& packet)
		{
			JE::Net::UInt8 bytes[8224];
			Buffer output(bytes, sizeof(bytes));
			packet.Serialize(output, 71);
			sockaddr_in destination;
			packet.address.ToSockaddr(destination);
			if (::sendto(handle, reinterpret_cast<const char*>(output.data), output.field0x10, 0,
				reinterpret_cast<const sockaddr*>(&destination), sizeof(destination)) == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error == WSAEWOULDBLOCK) return;
				if (error == WSAEADDRNOTAVAIL && destination.sin_addr.s_addr == INADDR_BROADCAST) return;
				throw SocketException("Could not send data on socket.");
			}
		}

		bool Socket::Receive(Packet& packet)
		{
			JE::Net::UInt8 bytes[8192];
			Buffer input(bytes, sizeof(bytes));
			sockaddr_in source;
			int size = sizeof(source);
			int received = ::recvfrom(handle, reinterpret_cast<char*>(bytes), sizeof(bytes), 0,
				reinterpret_cast<sockaddr*>(&source), &size);
			if (received == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK) return false;
				return false; // Original also suppresses every other receive error.
			}
			input.field0x10 = received;
			packet.Deserialize(input);
			packet.address.Assign(source); //Wire address is replaced with actual sender.
			return true;
		}
	}
}
