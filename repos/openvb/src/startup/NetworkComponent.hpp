#pragma once

#include "EngineRegistry.hpp"
#include "network/Peer.hpp"
#include "network/Types.hpp"
#include <cstddef>

namespace F3
{
	typedef JE::Net::Logger NetworkLogger;
	typedef JE::Net::Peer NetworkPeer;

	class NetworkComponent : public GameComponent
	{
	public:
		NetworkComponent(); // 0x543890
		~NetworkComponent(); // 0x543710
		void Initialize(); // 0x543A40
		void Cleanup(); // 0x543730
		void Update(); // 0x543770
		virtual NetworkPeer* CreateServer(); // 0x5437B0
		virtual NetworkPeer* CreateClient(); // 0x543820
		virtual void LoadConfiguration(); // 0x543AC0
		virtual void OnCleanup(); // 0x5441F0
		void SetMode(JE::Net::UInt32 requested); // 0x543930; incidental AL excluded.
		JE::Net::UInt32 GetMode() const; //5B4F30, actual shared mode getter.
		static void operator delete(void* block) throw();

		JE::Net::UInt32 mode;
		JE::Net::UInt16 serverPort;
		JE::Net::UInt32 serverDebug, serverRate, serverTimeout;
		NetworkPeer* server;
		JE::Net::UInt16 clientPort;
		JE::Net::UInt32 clientDebug, clientRate, clientTimeout;
		NetworkPeer* client;
	};
	class GameNetworkComponent : public NetworkComponent
	{
	public:
		virtual ~GameNetworkComponent(); //45E0D0, ordinary derived table phase.
		NetworkPeer* CreateServer(); // 0x45E040 -> 0x46EE70
		NetworkPeer* CreateClient(); // 0x45DFD0 -> 0x460B30
	};
	extern NetworkComponent* g_pNetworkInstance; // Original 0x70BD90.
	GameComponent* __cdecl CreateGameNetwork(); // 0x45E170.

	namespace NetworkComponentServices
	{
		void* Allocate(std::size_t size);
		void Free(void* block) throw();
		void InitializeRuntime();
		void ShutdownRuntime();
		void InvalidConfiguration();
		void StopWorldServer();
		void EnableServer(NetworkPeer& peer, unsigned char enabled);
		void DeletePeer(NetworkPeer* peer);
		void ClearObjects();
		void ClearTeams();
		void ClearSquads();
	}
}
