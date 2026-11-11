#include "Server.hpp"
#include "ConnectionFactory.hpp"

#include <new>
#include <stdexcept>

namespace JE
{
	namespace Net
	{
		Server::Server(JE::Net::UInt16 port) : Base(port), connections(NULL), enabled(0),
			field0x59(0), connectionCount(0), field0x60(0)
		{
			// Each slot constructs Address only; its other two DWORDs are unwritten.
		}

		Server::~Server() { Server::Cleanup(); }
		void Server::Initialize() { Configure(16); }
		void Server::PrepareDiscoveryResponse(Packet&) {} //5A9320.
		void Server::PrepareChallengeResponse(Packet&) {} //5A9320.
		void Server::PrepareConnectResponse(Packet&) {} //5A9320.
		JE::Net::UInt8 Server::OnDiscoveryRequest(Packet&) { return 1; } //409BB0.
		JE::Net::UInt8 Server::OnChallengeRequest(Packet&) { return 1; } //409BB0.
		JE::Net::UInt8 Server::OnConnectRequest(Packet&) { return 1; } //409BB0.

		void Server::CreateConnections()
		{
			for (JE::Net::UInt32 index = 0; index < connectionCount; ++index)
			{
				Connection* created = new ServerClientConnection;
				connections[index] = created;
			}
		}

		void Server::ResizeConnections(JE::Net::UInt32 count)
		{
			if (enabled) return;
			if (connections)
			{
				for (JE::Net::UInt32 index = 0; index < connectionCount; ++index)
				{
					Connection* current = connections[index];
					if (current) ServerServices::DestroyConnection(current);
				}
				ServerServices::FreeConnections(connections);
			}
			connectionCount = count;
			connections = ServerServices::AllocateConnections(count);
			CreateConnections();
			for (JE::Net::UInt32 index = 0; index < connectionCount; ++index)
			{
				ServerServices::SetConnectionLogger(connections[index], debugStream.logger);
				ServerServices::SetConnectionDebugFlags(connections[index], debugStream.debugFlags);
			}
		}

		void Server::DisconnectClient(JE::Net::UInt32 index, JE::Net::UInt8 notify)
		{
			if (static_cast<JE::Net::Int32>(connections[index]->state) > 0)
			{
				if (notify)
				{
					Packet packet;
					packet.address = connections[index]->address;
					packet.field0x30 = 8;
					char peer[64];
					debugStream.Print(16,"[server] Sending disconnect to client %u at %s\n",
						index,packet.address.Format(peer,sizeof(peer),1));
					PrepareDisconnect(index,packet);
					connections[index]->Send(packet,2);
				}
				connections[index]->Reset();
				--field0x60;
			}
		}

		void Server::SetEnabled(JE::Net::UInt8 requested)
		{
			if (enabled != requested)
			{
				if (enabled) DisconnectAll();
				enabled = requested;
			}
		}

		void Server::SetLogger(Logger callback)
		{
			debugStream.logger = callback;
			for (JE::Net::UInt32 index = 0; index < connectionCount; ++index)
				connections[index]->debugStream.logger = callback;
		}
		void Server::SetDebugFlags(JE::Net::UInt32 flags)
		{
			debugStream.debugFlags = flags;
			for (JE::Net::UInt32 index = 0; index < connectionCount; ++index)
				connections[index]->debugStream.debugFlags = flags;
		}

		void Server::Process()
		{
			if (!enabled) return;
			for (JE::Net::UInt32 index=0;index<connectionCount;++index)
				connections[index]->Process(); //5A08D0 reloads count after each call.
		}

		void Server::SendKeepalives() //5A18C0, no enabled test inside this helper.
		{
			for (JE::Net::UInt32 index=0;index<connectionCount;++index) SendKeepalive(index);
		}

		namespace ServerServices
		{
			void DestroyConnection(Connection* connection) { delete connection; }
			void FreeConnections(Connection** connections) { ::operator delete[](connections); }
			void Disconnect(Server& server, JE::Net::UInt32 index, JE::Net::UInt8 notify)
			{
				server.DisconnectClient(index, notify);
			}
			void SetConnectionLogger(Connection* connection, Logger callback)
			{
				connection->debugStream.logger = callback;
			}
			void SetConnectionDebugFlags(Connection* connection, JE::Net::UInt32 flags)
			{
				connection->debugStream.debugFlags = flags;
			}
			void ResizeConnections(Server& server, JE::Net::UInt32 count)
			{
				server.ResizeConnections(count);
			}
			Connection** AllocateConnections(JE::Net::UInt32 count)
			{
				return static_cast<Connection**>(::operator new[](static_cast<std::size_t>(count) * sizeof(Connection*)));
			}
			Connection* CreateClientConnection() { return ConstructConnection<ServerClientConnection>(); }
			void NotifyDisconnect(Server& server, JE::Net::UInt32 index)
			{
				server.SendDisconnect(index);
			}
		}
	}
}
