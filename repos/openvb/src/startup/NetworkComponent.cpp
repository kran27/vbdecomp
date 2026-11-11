#include "GameServer.hpp"
#include "GameClient.hpp"
#include "NetworkComponent.hpp"
#include "CommonException.hpp"
#include "FrameLookupCaches.hpp"
#include "network/NetworkRuntime.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "entity/World.hpp"

#include <new>
#include <stdexcept>

namespace F3
{
	JE::Net::UInt32 NetworkComponent::GetMode() const //5B4F30.
	{
		return mode;
	}

	NetworkComponent* g_pNetworkInstance = NULL;
	static const char g_szDuplicateNetwork[] = "Network object already exists.";

	NetworkComponent::NetworkComponent()
		: mode(0), serverPort(0x53FA), serverDebug(0), serverRate(100),
		serverTimeout(3000), server(NULL), clientPort(0x57E5), clientDebug(0),
		clientRate(10), clientTimeout(3000), client(NULL)
	{
		if (g_pNetworkInstance) throw Common::Exception(g_szDuplicateNetwork);
		g_pNetworkInstance = this;
	}
	NetworkComponent::~NetworkComponent() { g_pNetworkInstance = NULL; }
	void NetworkComponent::Initialize()
	{
		try { NetworkComponentServices::InitializeRuntime(); }
		catch (JE::Net::Exception error) { throw Common::Exception(error.what()); }
		LoadConfiguration();
		SetMode(mode);
	}
	void NetworkComponent::Cleanup()
	{
		OnCleanup();
		NetworkPeer* captured = client;
		if (captured)
		{
			client = NULL;
			NetworkComponentServices::DeletePeer(captured);
		}
		captured = server;
		if (captured)
		{
			server = NULL;
			NetworkComponentServices::DeletePeer(captured);
		}
		NetworkComponentServices::ShutdownRuntime();
	}
	void NetworkComponent::Update()
	{
		if (server)
		{
			server->Poll();
			server->Process(); // Reload after the callback; no favorable null guard.
		}
		if (client)
		{
			client->Poll();
			client->Process();
		}
		ClearFrameObjects();
		ClearFrameTeams();
		ClearFrameSquads();
	}
	void NetworkComponent::SetMode(JE::Net::UInt32 requested)
	{
		const JE::Net::UInt32 selected = requested & 3;
		if (selected != requested || !selected) NetworkComponentServices::InvalidConfiguration();
		if (selected & 1)
		{
			if (!server)
			{
				server = CreateServer();
				server->Initialize();
			}
			server->SetLogger(&JE::LogDebugString);
			server->SetDebugFlags(serverDebug);
			server->rate = serverRate;
			server->timeout = serverTimeout;
			NetworkComponentServices::EnableServer(*server, 1);
		}
		else if (server)
		{
			NetworkComponentServices::StopWorldServer();
			NetworkPeer* captured = server;
			if (captured) NetworkComponentServices::DeletePeer(captured);
			server = NULL; // Different ordering from Cleanup; retain it.
		}
		if (selected & 2)
		{
			if (!client)
			{
				client = CreateClient();
				client->Initialize();
			}
			client->SetLogger(&JE::LogDebugString);
			client->SetDebugFlags(clientDebug);
			client->rate = clientRate;
			client->timeout = clientTimeout;
		}
		else
		{
			NetworkPeer* captured = client;
			if (captured)
			{
				NetworkComponentServices::DeletePeer(captured);
				client = NULL;
			}
		}
		mode = selected;
	}
	GameComponent* __cdecl CreateGameNetwork()
	{
		return ::new GameNetworkComponent;
	}
	void NetworkComponent::operator delete(void* block) throw() { NetworkComponentServices::Free(block); }

	NetworkPeer* NetworkComponent::CreateServer() { return ::new JE::Server(serverPort); }
	NetworkPeer* NetworkComponent::CreateClient() { return ::new JE::Client(clientPort); }
	GameNetworkComponent::~GameNetworkComponent() {}
	NetworkPeer* GameNetworkComponent::CreateServer() { return ::new GameServer(serverPort); }
	NetworkPeer* GameNetworkComponent::CreateClient() { return ::new GameClient(clientPort); }

	namespace NetworkComponentServices
	{
		void* Allocate(std::size_t size) { return ::operator new(size); }
		void Free(void* block) throw() { ::operator delete(block); }
		void InitializeRuntime() { JE::Net::InitializeRuntime(); }
		void ShutdownRuntime() { JE::Net::ShutdownRuntime(); }
		void InvalidConfiguration() { JE::FatalError("Invalid network configuration requested.\r\n"); }
		void StopWorldServer() { JE::GetWorldServerInstance()->Reset(); } //5439AA..5439BD, actual IWorld+50.
		void EnableServer(NetworkPeer& peer, unsigned char enabled)
		{
			static_cast<JE::Net::Server&>(peer).SetEnabled(enabled);
		}
		void DeletePeer(NetworkPeer* peer) { delete peer; }
		void ClearObjects() { ClearFrameObjects(); }
		void ClearTeams() { ClearFrameTeams(); }
		void ClearSquads() { ClearFrameSquads(); }
	}
}
