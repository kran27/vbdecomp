#pragma once

#include <winsock2.h>
#include "Address.hpp"

namespace JE
{
	namespace Net
	{
		class Packet;
		struct SocketException : public Exception
		{
		public:
			explicit SocketException(const char* message) throw() : Exception(message) {}
		};

		class Socket
		{
		public:
			Socket() throw() : handle(INVALID_SOCKET) {}
			virtual ~Socket() { Close(); }
			int Setup(JE::Net::UInt16 port); // 0x5B8E60.
			int Close(); // 0x5B8D40.
			JE::Net::UInt16 GetAddress(Address& address); // 0x5B8D60.
			void Send(const Packet& packet); //5B8F80, real UDP sendto.
			bool Receive(Packet& packet); //5B9100, real UDP recvfrom.

			SOCKET handle;
		private:
			Socket(const Socket&);
			Socket& operator=(const Socket&);
		};

		namespace SocketServices
		{
			SOCKET Create(int family, int type, int protocol);
			int Ioctl(SOCKET socket, long command, u_long* value);
			int SetOption(SOCKET socket, int level, int option, const char* value, int size);
			int Bind(SOCKET socket, const sockaddr* address, int size);
			int Close(SOCKET socket);
			int HostName(char* name, int size);
			int SocketName(SOCKET socket, sockaddr* address, int* size);
		}
	}
}
