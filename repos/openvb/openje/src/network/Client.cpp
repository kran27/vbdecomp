#include "Client.hpp"
#include "ConnectionFactory.hpp"

#include <stdexcept>

namespace JE
{
	namespace Net
	{
		Client::Client(JE::Net::UInt16 port)
			: Base(port), connection(NULL), state(0), field0x68(0), field0x6C(0)
		{
		}

		void Client::Initialize()
		{
			Cleanup();
			CreateConnection();
			rate = 10;
			timeout = 3000;
		}

		void Client::Cleanup()
		{
			Disconnect();
			Connection* const current = connection;
			if (current) ClientServices::DestroyConnection(current);
			connection = NULL;
		}

		void Client::SetLogger(Logger callback)
		{
			debugStream.logger = callback;
			ClientServices::SetConnectionLogger(connection, callback);
		}

		void Client::SetDebugFlags(JE::Net::UInt32 flags)
		{
			debugStream.debugFlags = flags;
			ClientServices::SetConnectionDebugFlags(connection, flags);
		}

		void Client::CreateConnection()
		{
			connection = new ClientServerConnection;
		}

		void Client::Connect(const Address& address)
		{
			Disconnect();
			state = 1;
			serverAddress = address;
			field0x68 = 0;
		}

		void Client::Connect(const char* host)
		{
			Address address;
			address.Resolve(host);
			Connect(address);
		}

		void Client::Disconnect()
		{
			if (static_cast<JE::Net::Int32>(state) <= 1) state = 0;
			else
			{
				Packet packet;
				packet.address = connection->address;
				packet.field0x30 = 8;
				char peer[64];
				debugStream.Print(16,"[client %u] Sending disconnect to server at %s\n",
					address.port,packet.address.Format(peer,sizeof(peer),1));
				PrepareDisconnect(packet);
				Send(packet,2);
				connection->Reset();
				state = 0;
			}
		}

		void Client::Process() { connection->Process(); } //59C440, no NULL guard.

		void Client::RefreshServerLatency()
		{
			if (static_cast<JE::Net::Int32>(connection->state)>0) connection->RefreshLatency();
		}

		namespace ClientServices
		{
			Connection* CreateServerConnection()
			{
				return ConstructConnection<ClientServerConnection>();
			}
			void DestroyConnection(Connection* connection)
			{
				delete connection;
			}
			void SetConnectionLogger(Connection* connection, Logger callback)
			{
				connection->debugStream.logger = callback;
			}
			void SetConnectionDebugFlags(Connection* connection, JE::Net::UInt32 flags)
			{
				connection->debugStream.debugFlags = flags;
			}
			void DisconnectConnected(Client& client)
			{
				client.DisconnectProtocol();
			}
		}
	}
}
