#include <winsock2.h>
#include "EntityEnvironment.hpp"
#include "Entity.hpp"
#include "World.hpp"
#include "WorldCollections.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/MapMaskManager.hpp"
#include "startup/NetworkComponent.hpp"
#include "startup/GameClient.hpp"
#include "startup/EngineSettings.hpp"
#include "startup/MessageLoop.hpp"
#include <stdexcept>

namespace JE
{
	namespace
	{

		class NativeEntityEnvironment : public EntityEnvironment
		{
		public:
			virtual DWORD NetworkMode() { return F3::g_pNetworkInstance->GetMode(); }
			virtual Server* GetServer()
			{
				return static_cast<Server*>(F3::g_pNetworkInstance->server);
			}
			virtual void SetEntityTeam(Entity* entity, DWORD team)
			{
				GetServer()->AssignEntityTeam(entity->entityID, team);
			}
			virtual void SetGlobal(const std::string& name, LONG value)
			{
				GetServer()->SetGlobal(name, value);
			}
			virtual std::string PlayerResourceName()
			{
				return std::string(GetServer()->characterTemplate.c_str()); //551962, actual C-string boundary.
			}
			virtual void PublishPlayerID(DWORD id) { ClientOwner()->field0xE4 = id; }
			virtual DWORD ClientPlayerID() { return ClientOwner()->field0xE4; }
			virtual void ClearClientSelection(DWORD id)
			{
				Client* client = ClientOwner();
				if (client->field0xE0 == id) client->field0xE0 = 0; //551C50; preserve DC.
			}
			virtual DWORD FrameClock() { return F3::g_nElapsedTimeMS; }
			virtual const Configuration::Settings& EngineSettings() { return *F3::GetEngineSettings(); }
			virtual BYTE IsPartyMember(Entity* entity) { return g_World->GetParty()->Contains(entity); }
			virtual void BeforeWorldDestruction()
			{
				F3::GameClient* game = dynamic_cast<F3::GameClient*>(ClientOwner());
				if (game && game->Interfaces()->auxiliary24)
					throw std::logic_error("GameWorld46F21B: populated auxiliary24 texture owner430760 unresolved");
			}
			virtual void ClearGraphicsSelectionCache()
			{
				MapMaskManager* masks = GetMapMaskManager();
				if (masks) masks->Clear(); //581330, actual optional frontend child.
			}
			virtual MapMaskManager* GetMapMaskManager() { return ClientOwner()->child0x74->maskManager; }
			virtual DWORD LineOfSightMode() { return ClientOwner()->field0x100; }
			virtual OCTREE_Camera* CurrentMapCamera() { return ClientOwner()->child0x74->GetPickingCamera(); }
			virtual DWORD RenderMode() { return ClientOwner()->field0x104; }
			virtual DWORD WaterMode() { return ClientOwner()->field0x10C; }
		private:
			static Client* ClientOwner()
			{
				return static_cast<Client*>(F3::g_pNetworkInstance->client);
			}
		};
	}
	EntityEnvironment& GetEntityEnvironment()
	{
		static NativeEntityEnvironment native;
		return native;
	}
}
