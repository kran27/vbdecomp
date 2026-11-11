#include <winsock2.h>
#include "GameMap.hpp"
#include "GamePlayer.hpp"
#include "GameWorldCollections.hpp"
#include "entity/World.hpp"
#include "entity/EntityEnvironment.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "entity/MapFog.hpp"
#include <float.h>
#include <cmath>
#include <stdexcept>
namespace F3
{
	GameMap::GameMap() {}
	GameMap::~GameMap()
	{}
	void GameMap::GetCameraBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const
	{
		if(cameraConstraints==1)
		{
			JE::IWorld* world = JE::g_World;
			GameParty* party = dynamic_cast<GameParty*>(world->GetParty());
			JE::Entity* selected = party ? party->selectedEntity : NULL;
			if(selected)
			{
				high.y=0;
				low.y=0;
				const float x=selected->position.x,z=selected->position.z;
				const float minimumX=cameraLow.x,minimumZ=cameraLow.z;
				const float maximumX=cameraHigh.x,maximumZ=cameraHigh.z;
				low.x=x-50.0<minimumX ? minimumX : float(x-50.0);
				high.x=x+50.0>maximumX ? maximumX : float(x+50.0);
				low.z=z-50.0<minimumZ ? minimumZ : float(z-50.0);
				high.z=z+50.0>maximumZ ? maximumZ : float(z+50.0);
				return;
			}
		}
		low=D3DXVECTOR3(FLT_MIN,0,FLT_MAX);
		high=D3DXVECTOR3(FLT_MAX,0,FLT_MIN);
	}
	void GameMap::AddEntity(JE::Entity* entity)
	{
		JE::Map::AddEntity(entity);
		if(entity->entityID!=JE::GetEntityEnvironment().ClientPlayerID()) return;
		GamePlayer* player=dynamic_cast<GamePlayer*>(entity);
		if(!player) return;
		JE::AreaMap* cached;
		{
			//46CC13: key is Map+2C octreeName, copied through c_str/strlen.
			//The lookup's temporary string dies before any AreaMap destruction.
			std::string key(octreeName.c_str());
			cached=player->areaMaps->Find(key);
		}
		if(cached)
		{
			if(ownsAreaMap && areaMap) delete areaMap;
			areaMap=cached;
		}
		else
		{
			//46CC97 reloads the name, current AreaMap and player cache. Transfer
			//the actual owner; no clone, saved-map read or replacement is involved.
			std::string key(octreeName.c_str());
			JE::AreaMap* current=areaMap;
			player->areaMaps->Set(key,current);
		}
		ownsAreaMap=0;
	}
	void GameMap::RemoveEntity(JE::Entity* entity)
	{
		JE::Map::RemoveEntity(entity);
		if(entity->entityID==static_cast<JE::Client*>(g_pNetworkInstance->client)->field0xE4)
			if(dynamic_cast<GamePlayer*>(entity)) areaMap=NULL;
		//46CB1F does not erase the cache or change ownsAreaMap.
	}
	JE::Entity* GameMap::FindNearest(JE::Entity* from,const std::string& role)
	{
		JE::Entity* found=JE::Map::FindNearest(from,role);
		if(found)
		{
			double x=double(from->position.x)-found->position.x;
			double y=double(from->position.y)-found->position.y;
			double z=double(from->position.z)-found->position.z;
			x = x*x;
			x += z*z;
			x += y*y;
			float distance = float(sqrt(x));
			if(distance>500.0f) return NULL;
		}
		return found;
	}
}
