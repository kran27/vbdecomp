// Original calls use the WinSock import-library thunks.
#define WINSOCK_API_LINKAGE
#include "Socket.hpp"

namespace JE
{
	namespace Net
	{
		int Socket::Close()
		{
			int result = handle;
			if (handle != INVALID_SOCKET)
			{
				result = SocketServices::Close(handle);
				handle = INVALID_SOCKET;
			}
			return result;
		}

		int Socket::Setup(JE::Net::UInt16 port)
		{
			Close();
			const SOCKET candidate = SocketServices::Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (candidate == INVALID_SOCKET) throw SocketException("Could not create socket.");
			u_long value = 1;
			if (SocketServices::Ioctl(candidate, FIONBIO, &value) == SOCKET_ERROR)
				throw SocketException("Could not set socket to non-blocking.");
			if (SocketServices::SetOption(candidate, SOL_SOCKET, SO_BROADCAST,
				reinterpret_cast<const char*>(&value), sizeof(value)) == SOCKET_ERROR)
				throw SocketException("Could not enable broadcast on socket.");
			sockaddr_in address;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons(port);
			address.sin_family = AF_INET;
			// Original sin_zero bytes remain unwritten; WinSock does not use them.
			const int result = SocketServices::Bind(candidate,
				reinterpret_cast<const sockaddr*>(&address), sizeof(address));
			if (result == SOCKET_ERROR) throw SocketException("Could not bind socket.");
			// Do not add a local close guard: failed unpublished candidates leak.
			handle = candidate;
			return result;
		}

		JE::Net::UInt16 Socket::GetAddress(Address& address)
		{
			char name[256];
			int size = sizeof(name);
			if (SocketServices::HostName(name, size) == SOCKET_ERROR)
				throw SocketException("Unable to resolve host name.");
			address.Resolve(name);
			sockaddr_in native;
			size = sizeof(native);
			if (::getsockname(handle, reinterpret_cast<sockaddr*>(&native), &size)
				== SOCKET_ERROR)
				throw SocketException("Unable to resolve socket name.");
			address.port = ntohs(native.sin_port);
			return address.port;
		}

		namespace SocketServices
		{
			inline SOCKET Create(int family, int type, int protocol) { return ::socket(family, type, protocol); }
			inline int Ioctl(SOCKET socket, long command, u_long* value) { return ioctlsocket(socket, command, value); }
			inline int SetOption(SOCKET socket, int level, int option, const char* value, int size)
			{
				return setsockopt(socket, level, option, value, size);
			}
			inline int Bind(SOCKET socket, const sockaddr* address, int size) { return ::bind(socket, address, size); }
			inline int Close(SOCKET socket) { return closesocket(socket); }
			inline int HostName(char* name, int size) { return gethostname(name, size); }
			inline int SocketName(SOCKET socket, sockaddr* address, int* size) { return getsockname(socket, address, size); }
		}
	}
}
