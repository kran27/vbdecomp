#include "startup/NetworkComponent.hpp"
#include "network/EngineClientInitialize.hpp"
#include "graphics/VfxVeg.hpp"
#include "Map.hpp"
#include "World.hpp"
#include "network/EngineServer.hpp"
#include "EntityEnvironment.hpp"
#include "MapEffect.hpp"
#include "MapFog.hpp"
#include "Trigger.hpp"
#include "graphics/Octree.hpp"
#include "graphics/SystemInterface.hpp"
#include "resources/ResourceSystem.hpp"
#include "model/ModelServices.hpp"
#include "scripting/ScriptSmall.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "startup/CommonException.hpp"
#include <cstring>
#include <algorithm>
#include <float.h>
#include <cmath>
#include <stdexcept>

namespace JE
{
	VisualEffect* Map::AttachVisualEffect(const char* name, DWORD id, BYTE flag) //574F20.
	{
		float scale;
		std::memcpy(&scale, &EngineClientInitializationServices::g_SelectionEffectArgument, sizeof(scale));
		Map* owner = this;
		const char* resource = name;
		VFX_VEGInstance* instance = GetVfxVegCache().Acquire(resource, scale);
		if (!instance) return NULL;
		return owner->AttachVisualEffect(instance, 1, 1, id, flag);
	}

	WayPointPath* Map::FindWayPointPath(const std::string& name)
	{
		std::map<std::string,WayPointPath>::iterator found=waypoints.find(name);
		if (found!=waypoints.end()) return &found->second;
		return NULL;
	}

	void Map::SetEntryPoint(BYTE id, MapEntryPoint* point) { entryPoints[id] = point; }
	//Source factor of57B4DD..508: C++98 arrays initialize with the next real scalar.
	Map::Map() : areaMap(NULL), ownsAreaMap(0), resourceName(), value28(0), octreeName(), octree(NULL),
		gridName(), areaTextureName(), terrainGrid(NULL), entityGrid(NULL), alternateTerrain(NULL),
		scriptName(), script(NULL), scriptDisabled(0), idB4(0), state(0), entryPoints(), triggers(),
		waterTiles(), waterConfigurations(), waterObjects(), waterGroups(), waterSampleOwners(),
		visibilityOwners((color[0]=0xC0, color[1]=0xC0, color[2]=0xC0,
			ambient[0]=0xC0, ambient[1]=0xC0, ambient[2]=0xC0, ambient[3]=0x80, 0)),
		updateInterval(250), lastUpdateTime(0), nextUpdateTime(0), activeEntities(), entities(), musicName(),
		cameraConstraints(0), cameraLow(0,0,0), cameraHigh(0,0,0), flag178(0), value17C(DWORD(-1)),
		value180(1.5f), value184(0.16f), value188(0.25f), value18C(3.0f), value190(0.25f),
		value194(0.5f), value198(0), value19C(0.75f), flag1B1(0), fogColor(0xFF808080),
		fogNear(250.0f), fogFar(500.0f), fogScale(1.0f), flag1D8(1), flag1D9(1), mapNotes(),
		effects(), effectIDs(), inactiveEffects(), wind(0,0,0), hiddenMaterialOwners(), navigation(),
		waypoints(), mapSounds(), mapEffects()
	{
		//57B7BA..846 publishes backups only after every owned member constructs.
		value1A0=value17C;
		value1AC=value19C;
		value1A4=value180;
		value1A8=value18C;
		flag1B0=flag178;
		fogFlagCopy=flag1B1;
		fogColorCopy=float(fogColor);
		fogFarCopy=fogFar;
		fogNearCopy=fogNear;
		fogScaleCopy=fogScale;
		musicName.assign("MA1",3); //57B861, already-constructed empty string.
	}
	void Map::Create(const char* name)
	{
		if (state) throw ::Common::Exception("Create() method may only be called on invalid maps.");
		LogDebugString("\t+++ Map::Create( %s ) begin +++\n",name);
		idB4=g_World->AllocateMapID(); //57B92E, required actual shared IWorld owner.
		if (name && *name && !ReadResource(name))
			throw ::Common::Exception("Error reading map from Map::Create() method.");
		resourceName=name;
		if (scriptName.empty())
		{
			const char* extension=strrchr(name,'.');
			if (extension)
			{
				char resource[256];
				size_t prefix=extension-name+1;
				strncpy(resource,name,prefix);
				Resources::ResourceSystem* resources=Resources::g_pResourceSystem;
				strcpy(resource+prefix,"amx");
				if (resources->Contains(resource)) scriptName=resource;
			}
		}
		//57BA15..6E publishes the child before its owning name temporary dies.
		terrainGrid=new MapGrid(std::string(gridName.c_str()));
		entityGrid=new EntityGrid(terrainGrid->width,terrainGrid->height);
		float bounds[6]; GetOctree()->GetLevelBounds(bounds);
		float minimum[3]={bounds[0],bounds[1],bounds[2]};
		float maximum[3]={bounds[3],bounds[4],bounds[5]};
		UINT width=0,height=0;
		g_GfxSystem->CalculateMaskDimensions(minimum,maximum,&width,&height);
		//57BB3D..57BC57: distinct unused owner, retained through final logging.
		std::string unused;
		areaMap=new AreaMap(0,value28,std::string(areaTextureName.c_str()),
			bounds[0],bounds[2],bounds[3],bounds[5],width,height);
		ownsAreaMap=1; //After areaMap publication and owning argument cleanup.
		GenerateFogHeights();
		ChangeState(1);
		LogDebugString("Map %s contains %d entities,\n",name,int(entities.size()));
		LogDebugString("\t+++ Map::Create( %s ) end +++\n",name);
	}
	OCTREE* Map::GetOctree()
	{
		if (!octree)
		{
			Resources::ResourceFileHandle* file=Resources::g_pResourceSystem->OpenStream(octreeName.c_str(),1);
			if (OCTREE::Read8Resource(file,7936,&octree))
			{
				Resources::g_pResourceSystem->Release(file,0);
				FatalError("Map::GetOctree() - Unable to load octree %s.",octreeName.c_str());
			}
			OCTREE_Scene* scene=NULL;
			int mode=1;
			do
			{
				if (octree->CreateScene(&scene,mode)!=2) break;
			}
			while (mode--);
			if (scene) scene->Release();
			else FatalError("Map::GetOctree() - Unable to initialize octree %s.",octreeName.c_str());
			Resources::g_pResourceSystem->Release(file,0);
		}
		return octree;
	}
	DWORD Map::GetUpdateInterval() const { return updateInterval; }
	void Map::SetUpdateInterval(DWORD interval) { updateInterval=interval; }
	BYTE Map::IsPeriodic() const { return GetUpdateInterval() > 0 ? 1 : 0; }
	BOOL Map::NeedsUpdate() const { return IsPeriodic() || state==1 || state==2; }
	BYTE Map::UpdateDue(DWORD time) const { return state==1 || state==2 || (IsPeriodic() && nextUpdateTime<=time); }
	void Map::ChangeState(DWORD next)
	{
		if (state==0 && next!=1 && next!=2)
			throw ::Common::Exception("Invalid state change in Map::ChangeState().");
		if ((state==1 || state==2) && next!=3)
			throw ::Common::Exception("Invalid state change in Map::ChangeState().");
		if (state==3)
			throw ::Common::Exception("Invalid state change in Map::ChangeState().");
		state=next;
	}
	void Map::OnUpdate(DWORD time)
	{
		if (state!=3) throw ::Common::Exception("Map is not in a valid state to be updated.");
		IWorld* world=g_World;
		DWORD cutscene=world->GetCutsceneState();
		cell result=0;
		if (static_cast<Server*>(F3::g_pNetworkInstance->server)->field0x70E4)
		{
			if (cutscene)
			{
				cell arguments[2]; arguments[0]=cell(time);
				world=g_World;
				arguments[1]=cell(world->GetCutsceneID());
				switch(cutscene)
				{
				case 1: RunScript("OnCutsceneStart",&result,2,arguments); break;
				case 2: RunScript("OnCutscene",&result,2,arguments); break;
				case 3: RunScript("OnCutsceneEnd",&result,2,arguments); break;
				}
			}
			else
			{
				cell arguments[1]; arguments[0]=cell(time);
				RunScript("OnUpdate",&result,1,arguments);
			}
		}
		for (std::list<Trigger*>::iterator i=triggers.begin(); i!=triggers.end();)
		{
			Trigger* trigger=*i;
			trigger->Update(time);
			if (trigger->IsDestroyed()) { i=triggers.erase(i); delete trigger; }
			else ++i;
		}
	}
	BYTE Map::RunScript(const char* name,cell* result,int count,cell* args)
	{
		if (!(F3::g_pNetworkInstance->GetMode()&1))
			throw ::Common::Exception("Map::RunScript() method may only be called by the server.");
		if (scriptDisabled) return 0;
		if (!script)
		{
			if (scriptName.empty()) return 0;
			IWorldServer* server = GetWorldServerInstance();
			Script_Small* created = server->NewScript(idB4);
			script = created;
			if (!created->Load(scriptName.c_str(),1))
			{
				Script_Small* failed = script;
				scriptDisabled = 1;
				delete failed;
				script = NULL;
				return 0;
			}
		}
		return script->Run(name,result,count,args);
	}
	void Map::GetCameraBounds(D3DXVECTOR3& low,D3DXVECTOR3& high) const
	{
		if (cameraConstraints == 1)
		{
			DWORD word;
			std::memcpy(&word,&cameraLow.x,sizeof(word)); std::memcpy(&low.x,&word,sizeof(word));
			std::memcpy(&word,&cameraLow.y,sizeof(word)); std::memcpy(&low.y,&word,sizeof(word));
			std::memcpy(&word,&cameraLow.z,sizeof(word)); std::memcpy(&low.z,&word,sizeof(word));
			std::memcpy(&word,&cameraHigh.x,sizeof(word)); std::memcpy(&high.x,&word,sizeof(word));
			std::memcpy(&word,&cameraHigh.y,sizeof(word)); std::memcpy(&high.y,&word,sizeof(word));
			std::memcpy(&word,&cameraHigh.z,sizeof(word)); std::memcpy(&high.z,&word,sizeof(word));
		}
		else
		{
			low.x=FLT_MIN; low.y=0; low.z=FLT_MAX;
			high.x=FLT_MAX; high.y=0; high.z=FLT_MIN;
		}
	}
	Entity* Map::FindNearest(Entity* from,const std::string& role)
	{
		Entity* result=NULL;
		float nearest=FLT_MAX;
		for(std::list<Entity*>::iterator i=activeEntities.begin();i!=activeEntities.end();++i)
		{
			Entity* candidate=*i;
			if(candidate==from || candidate->roleName!=role) continue;
			double x=double(from->position.x)-candidate->position.x;
			double y=double(from->position.y)-candidate->position.y;
			double z=double(from->position.z)-candidate->position.z;
			double distance=sqrt(x*x+z*z+y*y);
			if(!result || distance<nearest) { nearest=float(distance); result=candidate; }
		}
		return result;
	}
	void Map::AddEntity(Entity* entity)
	{
		if(!state) throw ::Common::Exception("Map is not in a valid state to add entities.");
		if (F3::g_pNetworkInstance->GetMode()&1)
			for (std::list<Trigger*>::iterator i=triggers.begin(); i!=triggers.end(); ++i) (*i)->AddEntity(entity);
	}
	void Map::RemoveEntity(Entity* entity)
	{
		if(!state) throw ::Common::Exception("Map is not in a valid state to remove entities.");
		if(F3::g_pNetworkInstance->GetMode()&1)
		{
			for (std::list<Trigger*>::iterator i=triggers.begin(); i!=triggers.end(); ++i) (*i)->RemoveEntity(entity);
			entity->ClearMapMessages();
			entity->ClearMapAssociations();
		}
	}
	void Map::Invalidate(Entity* entity)
	{
		if(!state) throw ::Common::Exception("Map is not in a valid state to move entities.");
		if (F3::g_pNetworkInstance->GetMode()&1)
			for (std::list<Trigger*>::iterator i=triggers.begin(); i!=triggers.end(); ++i) (*i)->MoveEntity(entity);
	}
	void Map::AttachEntity(Entity* entity)
	{
		if(entity->mapHandle==idB4) return;
		if(entity->mapHandle)
		{
			IWorld* world = g_World;
			Map* old = world->GetMap(entity->mapHandle);
			old->DetachEntity(entity);
		}
		activeEntities.push_back(entity);
		AssignEntityMap(entity);
		AddEntity(entity);
	}
	void Map::DetachEntity(Entity* entity)
	{
		std::list<Entity*>::iterator first = activeEntities.begin();
		std::list<Entity*>::iterator end = activeEntities.end();
		std::list<Entity*>::iterator found = std::find(first, end, entity);
		if (found != end)
		{
			RemoveEntity(entity);
			activeEntities.erase(found);
			ClearEntityMap(entity);
		}
	}
	void Map::AssignEntityMap(Entity* entity)
	{
		entity->SetMap(idB4);
	}
	void Map::ClearEntityMap(Entity* entity)
	{
		entity->SetMap(0);
	}

	BYTE Map::ReadEntity(Common::Chunk&, Resources::ResourceMemoryStream* stream)
	{
		Entity* entity;
		{
			std::string resource;
			if (!Common::ReadString(resource, stream)) return 0;
			IWorld* world = g_World;
			DWORD type = world->ClassifyEntity(resource, 0);
			world = g_World;
			entity = world->NewEntity(type);
			D3DXVECTOR3 position(0,0,0);
			if (!Resources::g_pResourceSystem->ReadMemory(&position.x, 4, stream))
			{
				if (entity) delete entity;
				return 0;
			}
			if (!Resources::g_pResourceSystem->ReadMemory(&position.y, 4, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&position.z, 4, stream))
			{
				if (entity) delete entity;
				return 0;
			}
			D3DXVECTOR3 angles(0,0,0);
			if (!Resources::g_pResourceSystem->ReadMemory(&angles.x, 4, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&angles.y, 4, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&angles.z, 4, stream))
			{
				if (entity) delete entity;
				return 0;
			}
			angles.x *= 57.295776f; angles.y *= 57.295776f; angles.z *= 57.295776f;
			BYTE value;
			if (!Resources::g_pResourceSystem->ReadMemory(&value, 1, stream))
			{
				if (entity) delete entity;
				return 0;
			}
			entity->Create(resource.c_str(), value != 0, 1);
			entity->SetPosition(position);
			entity->SetRotation(angles);
			entity->renderEnabled = value != 0;
			Common::Chunk overrides;
			if (!overrides.Read(stream) || !entity->ReadEEOV(overrides, stream))
			{
				delete entity;
				return 0;
			}
			overrides.SkipToEnd();
			std::string texture(entity->GetTextureOverrideName()); //409CB0 returns a borrowed C string.
			if (!texture.empty()) entity->RefreshTexture();
		} //574221..57423D destroy texture, Chunk and resource before insertion.
		entities.insert(entities.end(), entity);
		return 1;
	}
}
