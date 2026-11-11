#include "GameServer.hpp"
#include "../game/GameEntity.hpp"
#include "EngineSettings.hpp"
#include "GameClient.hpp"
#include "NetworkComponent.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "graphics/VfxVeg.hpp"

#include <cstdlib>

namespace F3
{
	GameServer::GameServer(JE::Net::UInt16 port) : JE::Server(port)
	{
		const char* borrowedStartMap = NULL;
		JE::Net::UInt8 entry = 0;
		JE::Configuration::Settings* capturedSettings = GetEngineSettings();
		JE::Configuration::Settings::iterator group = capturedSettings->find("Server");
		if (group != capturedSettings->end())
		{
			const JE::Configuration::Values& capturedValues = group->second;
			JE::Configuration::Values::const_iterator map = capturedValues.find("Start map");
			if (map != capturedValues.end()) borrowedStartMap = map->second.c_str();
			JE::Configuration::Values::const_iterator value = capturedValues.find("Start map entry point");
			if (value != capturedValues.end())
				entry = static_cast<JE::Net::UInt8>(std::atol(value->second.c_str()));
		}
		if (borrowedStartMap)
		{
			startMap.assign(borrowedStartMap);
			entryPoint = entry;
		}
		else JE::FatalError("No start map specified in sys.ini");
		child0x70F0 = NULL; field0x70F4 = 0;
		child0x70F8 = NULL; field0x70FC = 0;
		child0x7100 = NULL; field0x7104 = 0;
	}

	GameServer::~GameServer()
	{
		// 0x46CE90 calls only the qualified base destructor. Explicit Cleanup
		// owns the three derived child deletions; do not invoke it here.
	}

	void GameServer::Cleanup()
	{
		if (child0x70F0)
		{
			GameServerServices::DeleteChild(child0x70F0);
			child0x70F0 = NULL;
		}
		if (child0x70F8)
		{
			GameServerServices::DeleteChild(child0x70F8);
			child0x70F8 = NULL;
		}
		if (child0x7100)
		{
			GameServerServices::DeleteChild(child0x7100);
			child0x7100 = NULL;
		}
		JE::Server::Cleanup();
	}

	void GameServer::HideInterfaces() //46EDF0, packet tail46D560 and EH cleanup630870.
	{
		static_cast<GameClient*>(g_pNetworkInstance->client)->HideInterfaces();
		SendInterfaceHide();
	}

	void GameServer::SendInterfaceHide() //46D560, full original packet lifetime.
	{
		JE::Net::Packet notification;
		notification.field0x30 = 104;
		Broadcast(notification, 1);
		JE::LogDebugString("Server - Sending GAME_ID_INTERFACE_HIDE\n");
	}

	LONG GameServer::SetEntityHealth(DWORD id, LONG health)
	{
		JE::Entity* owner = JE::GetEntityPtr(id);
		if (!owner) return 0;
		GameEntity* entity = dynamic_cast<GameEntity*>(owner);
		if (!entity) return 0;
		entity->SetHealth(health);
		SendEntityHealth(id, health);
		return 1;
	}

	namespace GameServerServices
	{
		JE::Configuration::Settings* GetSettings() { return GetEngineSettings(); }
		const JE::Configuration::Values* FindSection(const JE::Configuration::Settings& settings,
			const char* name)
		{
			JE::Configuration::Settings::const_iterator found = settings.find(name);
			return found == settings.end() ? NULL : &found->second;
		}
		const std::string* FindValue(const JE::Configuration::Values& values, const char* name)
		{
			JE::Configuration::Values::const_iterator found = values.find(name);
			return found == values.end() ? NULL : &found->second;
		}
		long ToLong(const char* value) { return std::atol(value); }
		void AssignString(std::string& target, const char* value) { target.assign(value); }
		void MissingStartMap() { JE::FatalError("No start map specified in sys.ini"); }
		void DeleteChild(JE::VFX_VEGInstance* child)
		{
			//46CEB0 invokes the actual deleting destructor, not cache.Release.
			//The VEG cache's data reference count therefore remains unchanged.
			delete child;
		}
	}
}
