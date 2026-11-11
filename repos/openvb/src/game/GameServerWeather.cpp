#include "GameServerWeather.hpp"
#include "startup/GameServer.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "graphics/VfxVeg.hpp"
#include "network/Packet.hpp"

namespace F3
{
	namespace
	{
		void ToggleWeatherEffect(JE::VFX_VEGInstance*& instance,
			JE::VisualEffect*& effect, const char* resource)
		{
			//41D050 -> IWorld+5C returns the current active map ID. The second
			//World getter and GetMap occur before lazy VEG acquisition.
			DWORD mapID = JE::g_World->valueE0;
			JE::Map* map = JE::g_World->GetMap(mapID);
			if (!instance)
			{
				const float scale = 10.0f; //6FF0D8, captured before cache getter.
				JE::VFX_VEGCache& cache = JE::GetVfxVegCache();
				instance = cache.Acquire(resource, scale);
			}
			if (map && instance)
			{
				if (effect)
				{
					map->RemoveVisualEffect(effect, 1);
					effect = NULL;
				}
				else
				{
					//Map owns the VisualEffect, borrowing this server-owned instance.
					effect = map->AttachVisualEffect(instance, 0, 0, 0, 1);
				}
			}
		}
	}

	void GameServer::ToggleWeather(DWORD message)
	{
		switch (message)
		{
		case 110:
			ToggleWeatherEffect(child0x70F0, field0x70F4, "demo_rain.veg");
			break;
		case 111:
			ToggleWeatherEffect(child0x70F8, field0x70FC, "demo_snow.veg");
			break;
		case 112:
			ToggleWeatherEffect(child0x7100, field0x7104, "demo_wind.veg");
			break;
		}
	}

	BYTE ReceiveGameServerWeatherRequest(GameServer& server, JE::Net::Packet& packet)
	{
		if (packet.field0x30 < 110 || packet.field0x30 > 112) return 0;
		server.ToggleWeather(packet.field0x30);
		return 1; //Adapter consumption flag; original branch returns void.
	}
}
