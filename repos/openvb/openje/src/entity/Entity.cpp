#include "startup/NetworkComponent.hpp"
#include "network/EngineClientInitialize.hpp"
#include "network/EngineServer.hpp"
#include "Entity.hpp"
#include "model/ModelCache.hpp"
#include "model/ModelInstance.hpp"
#include "Inventory.hpp"
#include "EffectTreeTrigger.hpp"
#include "EntityEnvironment.hpp"
#include "SpatialFootprint.hpp"
#include "World.hpp"
#include "Map.hpp"
#include "MapGrid.hpp"
#include "PathManager.hpp"
#include "graphics/Octree.hpp"
#include "graphics/ModelInstance.hpp"
#include "Combat.hpp"
#include "model/CommonException.hpp"
#include "VisualEffect.hpp"
#include "scripting/ScriptSmall.hpp"
#include "graphics/VfxVeg.hpp"
#include "resources/ResourceSystem.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <float.h>
#include <list>
#include <mbstring.h>
#include <string.h>
#include <stdexcept>

namespace JE
{
	void Entity::EnterCombat() //5490B0.
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		static_cast<IWorldServer*>(g_World)->GetCombat()->AddEntity(this);
	}

	void Entity::LeaveCombat() //551C80.
	{
		if (state == 5) ChangeState(4);
	}

	void Entity::Create(const char* resource, BYTE loadModel, BYTE skipDefaultOverride)
	{
		const char* label = resource && *resource ? resource : "<generic>";
		if (state) throw ::Common::Exception("Create() method may only be called on invalid entities.");
		LogDebugString("\t+++ Entity::Create( %s ) begin +++\n", label);
		IWorldClient* worldClient = g_World;
		IWorld* world = worldClient;
		entityID = world->AllocateEntityID(); //551909, actual shared IWorld+24.
		Inventory* createdInventory = CreateInventory();
		DWORD createdID = entityID; //55191E captures the live ID after the factory.
		inventory = createdInventory;
		createdInventory->ownerEntityID = createdID;
		if (resource && *resource)
		{
			ReadResource(resource, skipDefaultOverride); // Original ignores its BYTE result.
			resourceName = resource;
		}
		bool isPlayer;
		{
			std::string player = static_cast<Server*>(F3::g_pNetworkInstance->server)->characterTemplate.c_str();
			isPlayer = resourceName.compare(0, resourceName.size(), player.c_str(), player.size()) == 0;
		} //5519F4..551A1D destroys the name before player/model callbacks.
		if (isPlayer)
		{
			if (scriptName.empty()) scriptName = "Player.amx";
			static_cast<Client*>(F3::g_pNetworkInstance->client)->field0xE4 = entityID;
		}
		if (loadModel)
		{
			SetModel(modelName.c_str(), 0);
			OnModelLoaded();
			modelAttempted = 1;
		}
		else
		{
			modelMaximum.x = 5.0f;
			modelMaximum.y = 5.0f;
			modelMaximum.z = 5.0f;
			modelMinimum.x = -5.0f;
			modelMinimum.y = -5.0f;
			modelMinimum.z = -5.0f;
			minimum = modelMinimum;
			maximum = modelMaximum;
		}
		OnCreated();
		if (F3::g_pNetworkInstance->GetMode() & 1) spatialFootprint = CreateSpatialFootprint();
		ChangeState(1);
		LogDebugString("\t+++ Entity::Create( %s ) end +++\n", label);
	}

	ContextMenuEntry::~ContextMenuEntry() {} //52AFB0, shared string-member cleanup.

	void Entity::OnAnimationUpdateReceived(DWORD,DWORD) {} //Original+D4,408090.

	namespace
	{
		DWORD nextVisualEffectID; // Original loader-zero70BE60, allocator552A20.
	}
	std::list<Entity*> g_createdEntities; //70BE08, same owner/init order; head70BE0C.
	namespace
	{
		std::map<DWORD,Entity*> registeredEntities; // 70BE14; inventory publication54ED50.
		std::list<Entity*> updateEntities; // 70BE20, borrowed update membership.
		BYTE updateQueueChanged; // 70BDE8, loader-zero.
	}

	void ProcessEntityCreationList(void (*deleteEntity)(Entity*))
	{
		std::list<Entity*>::iterator i = g_createdEntities.begin();
		while (i != g_createdEntities.end())
		{
			Entity* entity = *i;
			if (entity->state == 8)
			{
				i = g_createdEntities.erase(i); //545D95..545DBE unlinks before Server5555A0.
				if (!deleteEntity)
					throw std::logic_error("World545D60: required Server retirement5555A0 unresolved");
				deleteEntity(entity);
			}
			else
			{
				if (entity->associationsDirty) entity->RefreshAssociations();
				++i; //545DE0 advances the held node even if the callback erased it.
			}
		}
	}

	void DeleteAllEntities()
	{
		//Destructors remove their canonical creation node and may also remove
		//other nodes. Original reset reloads the live head after every deletion.
		while (!g_createdEntities.empty()) delete g_createdEntities.front();
	}

	void UpdateEntities(World& world)
	{
		for (std::list<Entity*>::iterator i = updateEntities.begin(); i != updateEntities.end(); ++i)
		{
			Entity* entity = *i;
			if (entity->state == 5) continue;
			entity->UpdateClient(world.GetServerTime());
			entity->ServerUpdate(world.GetServerTime());
			//545EC7 still advances the held iterator after either callback erases it.
		}
	}

	void World::UpdateServerWorld(DWORD worldTime)
	{
		std::list<Entity*>::iterator created = g_createdEntities.begin();
		while (created != g_createdEntities.end())
		{
			Entity* entity = *created;
			if (entity->state == 8)
			{
				created = g_createdEntities.erase(created);
				static_cast<Server*>(F3::g_pNetworkInstance->server)->DeleteEntity(entity);
			}
			else
			{
				if (entity->associationsDirty) entity->RefreshAssociations();
				++created;
			}
		}
		const bool updateCutscene = cutscene154->state != 0;
		combat->Update(GetServerTime());
		paths->Update();
		if (combat->active) return;
		for (WorldMaps::iterator map = maps.begin(); map != maps.end(); ++map)
		{
			Map* current = map->second;
			current->Update(GetServerTime());
		}
		for (std::list<Entity*>::iterator current = updateEntities.begin(); current != updateEntities.end(); ++current)
		{
			Entity* entity = *current;
			if (entity->state == 5) continue;
			entity->UpdateClient(GetServerTime());
			entity->ServerUpdate(GetServerTime());
		}
		if (!combat->actors.empty() && combat->mode)
		{
			SetServerTime(GetServerTime() + 1000 * value40);
			calendar->AddSeconds(value40);
		}
		else
		{
			float elapsed = static_cast<float>(worldTime - value68);
			float scaled = GetTimeScale() * elapsed;
			SetServerTime(GetServerTime() + static_cast<DWORD>(static_cast<__int64>(scaled)));
			value44 = static_cast<DWORD>(static_cast<__int64>(value44 + scaled));
			if (value44 >= 1000)
			{
				calendar->AddSeconds(value44 / 1000);
				value44 %= 1000;
			}
		}
		if (updateCutscene) cutscene154->Update();
		static_cast<Server*>(F3::g_pNetworkInstance->server)->SetScriptStep(0);
	}

	DWORD AllocateVisualEffectID()
	{
		if (!++nextVisualEffectID) nextVisualEffectID = 1;
		return nextVisualEffectID;
	}

	DWORD Server::AllocateVisualEffectID()
	{
		if (!++nextVisualEffectID) nextVisualEffectID = 1;
		return nextVisualEffectID; // Same counter; native callers supply the Server receiver.
	}

	Entity* GetEntityPtr(DWORD id)
	{
		Entity* result = NULL;
		for (std::list<Entity*>::const_iterator i = g_createdEntities.begin(); i != g_createdEntities.end(); ++i)
		{
			Entity* entity = *i;
			if (entity->entityID == id) return entity;
		}
		std::map<DWORD,Entity*>::iterator found;
		found = registeredEntities.find(id);
		if (found != registeredEntities.end()) return found->second;
		return result;
	}

	Entity* FindEntityByName(std::string name)
	{
		Entity* result = NULL;
		for (std::list<Entity*>::const_iterator i = g_createdEntities.begin(); i != g_createdEntities.end(); ++i)
		{
			Entity* entity = *i;
			if (!_stricmp(entity->displayName.c_str(), name.c_str()))
			{
				result = entity;
				break;
			}
		}
		return result;
	}

	void VisitEntities(void (*visitor)(Entity*,void*), void* context)
	{
		for (std::list<Entity*>::iterator i = g_createdEntities.begin(); i != g_createdEntities.end(); ++i)
			visitor(*i, context);
	}

	Entity::Entity()
		: DynamicObject(), model(NULL), modelAttempted(0), inventory(NULL), spatialFootprint(NULL),
		inFootprintGrid(0), associationsDirty(0), actionBudget(0), enabled(1), shadow(1), flag17A(0),
		value17C(0), animationStartTime(0), animationEndTime(0), animationLooping(0),
		resourceName(), animationQueue(), pendingAnimation(0), displayName(),
		entityID(0), state(0), visibilityOwnerFlag(0), inUpdateQueue(0), mapHandle(0), rotation(0,0,0),
		roleName(), modelName(), textureOverride(), inventoryTexture(), activeTexture(),
		animationEnabled(1), currentAnimationGroup(DWORD(-1)), currentAnimationIndex(0),
		animationReverse(0), suppressIdleAnimation(0), flag296(1), frozenAnimation(0),
		frozenAnimationTime(0), value29C(0), scriptName(), script(NULL), scriptDisabled(0), variables(),
		updateInterval(0), lastUpdateTime(0), nextUpdateTime(0), inventoryOwnerID(0), effects(), lights(),
		frameTime(0), actions(), actionAlternating(0), actionBlocked(0), value31C(0),
		mapMessages(), mapAssociations(), activeEffects(), effectIDs(), inactiveEffects(),
		lastWaterPosition(0,0,0), value368(0), value36C(0), squadID(0), squadRank(0),
		messages(), currentMessage(NULL), selectable(1), value908(DWORD(-1)), value912(DWORD(-1)),
		scriptValues(), renderEnabled(1), dialogueEnabled(1), moving(0),
		modelMinimum(minimum), modelMaximum(maximum), count956(1), activeSounds(), inactiveSounds()
	{
		//551813..551851: publish only after constructing all owned members.
		//entityClass and alignment padding remain unwritten by this constructor.
		g_createdEntities.push_back(this);
	}

	Entity::~Entity()
	{
		if (F3::g_pNetworkInstance->GetMode() & 1)
		{
			//55002E..55003D:401150 checks mode again, then dispatch through IWorldServer.
			GetWorldServerInstance()->GetCombat()->RemoveEntity(this);
		}
		if (inventory) delete inventory;
		if (inventoryOwnerID)
		{
			Entity* owner = GetEntityPtr(inventoryOwnerID);
			owner->inventory->Remove(this); //550070, after deleting our own inventory.
		}
		ClearSounds(); //550076 ->54C180, before the visual-effect owners.
		ClearVisualEffects();
		if (mapHandle)
		{
			IWorld* world = g_World;
			Map* map = world->GetMap(mapHandle); //5500AE, captured for this whole block.
			for (std::list<EntityLight*>::iterator light = lights.begin(); light != lights.end(); ++light)
				map->triggers.remove(*light);
			while (!lights.empty())
			{
				EntityLight* light = lights.front();
				lights.pop_front();
				delete light;
			}
			map->DetachEntity(this); //550165, removes active-map node and calls SetMap(0).
		}
		if (script) delete script;
		if (model) g_pModelCache->ReleaseModelInstance(model); //55018B leaves model unwritten.
		if (spatialFootprint) delete spatialFootprint;
		ClearEffects(); //5501A2, original precedes ClearActions.
		ClearActions();
		DiscardMessage(); //5501B0 invokes54B3F0 once; remaining payload leaks are original.
		RemoveFromUpdateQueue();
		g_createdEntities.remove(this);
	}

	void Entity::AddToUpdateQueue()
	{
		if (inUpdateQueue) return;
		inUpdateQueue = 1;
		updateEntities.push_back(this);
		updateQueueChanged = 1;
	}

	void Entity::RemoveFromUpdateQueue()
	{
		if (!inUpdateQueue) return;
		inUpdateQueue = 0;
		updateEntities.remove(this);
		updateQueueChanged = 1;
	}

	void Entity::Destroy()
	{
		if (!state) throw ::Common::Exception("Destroy() method may only be called on valid entities.");
		ChangeState(2);
	}

	void Entity::SetCount(DWORD value)
	{
		if (!value)
		{
			count956 = 1;
			return;
		}
		count956 = value;
	}

	void Entity::AttachEffectsToMap(DWORD map)
	{
		if (!map) return;
		IWorld* world = GetWorldInstance();
		Map* owner = world->GetMap(map);
		if (!owner) return;
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = activeEffects.begin();
			i != activeEffects.end(); ++i) i->second.object->AttachToOctree(owner->GetOctree());
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = inactiveEffects.begin();
			i != inactiveEffects.end(); ++i) i->second.object->AttachToOctree(owner->GetOctree());
	}

	void Entity::DetachEffectsFromMap()
	{
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = activeEffects.begin();
			i != activeEffects.end(); ++i) i->second.object->RemoveFromOctree();
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = inactiveEffects.begin();
			i != inactiveEffects.end(); ++i) i->second.object->RemoveFromOctree();
	}

	void Entity::SetMap(DWORD value)
	{
		if (!value)
		{
			RemoveFromOctree();
			if (mapHandle)
			{
				Map* old = GetWorldInstance()->GetMap(mapHandle);
				if (F3::g_pNetworkInstance->GetMode() & 1)
				{
					RemoveFootprint(old->entityGrid);
				}
				for (std::list<EntityLight*>::iterator light = lights.begin(); light != lights.end(); ++light)
				{
					EntityLight* held = *light;
					old->UnregisterTrigger(held);
					held->mapID = 0;
				}
			}
			DetachEffectsFromMap();
		}
		if (visibilityOwnerFlag)
		{
			if (mapHandle)
			{
				Map* old = GetWorldInstance()->GetMap(mapHandle);
				--old->visibilityOwners;
			}
			if (value)
			{
				Map* next = GetWorldInstance()->GetMap(value);
				++next->visibilityOwners;
			}
		}
		mapHandle = value;
		if (value)
		{
			Map* owner = GetWorldInstance()->GetMap(value);
			if (enabled == 1 && (!model || !model->IsAttached())) SnapToFloor();
			if (!inventoryOwnerID)
			{
				AttachToOctree(owner->GetOctree());
				if (F3::g_pNetworkInstance->GetMode() & 1) AddFootprint(owner->entityGrid);
				AttachEffectsToMap(value);
			}
			for (std::list<EntityLight*>::iterator light = lights.begin(); light != lights.end(); ++light)
			{
				EntityLight* held = *light;
				if (held->mapID != value)
				{
					owner->RegisterTrigger(held);
					held->mapID = value;
				}
			}
		}
		value912 = ~value912;
		value908 = value912;
	}

	void Entity::AddFootprint(RLEGrid<Entity*>* grid)
	{
		if (inFootprintGrid) return;
		grid->AddFootprint(spatialFootprint);
		inFootprintGrid = 1;
	}

	void Entity::RemoveFootprint(RLEGrid<Entity*>* grid)
	{
		if (!inFootprintGrid) return;
		RLEPrimitive<Entity*>* primitive = spatialFootprint;
		for (LONG row = 0; row < grid->height; ++row)
			grid->RemoveFromRow(row, primitive->value);
		inFootprintGrid = 0;
	}

	void Entity::SetInventoryOwner(DWORD id)
	{
		if (inventoryOwnerID)
		{
			if (!id)
			{
				const DWORD key=entityID;
				registeredEntities.erase(key);
				g_createdEntities.push_back(this);
			}
		}
		else if (id)
		{
			g_createdEntities.remove(this);
			registeredEntities[entityID] = this;
		}
		inventoryOwnerID = id;
	}

	VisualEffect* Entity::AttachVisualEffect(const char* name, DWORD id, BYTE bindingEnabled)
	{
		float scale;
		memcpy(&scale, &EngineClientInitializationServices::g_SelectionEffectArgument, sizeof(scale));
		VFX_VEGInstance* instance = GetVfxVegCache().Acquire(name, scale);
		return AttachVisualEffect(instance, 1, 1, id, bindingEnabled);
	}

	VisualEffect* Entity::AttachVisualEffect(VFX_VEGInstance* instance, BYTE release,
		BYTE own, DWORD id, BYTE bindingEnabled)
	{
		if (!instance) return NULL;
		std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = activeEffects.find(instance);
		if (i == activeEffects.end())
		{
			std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator old;
			old = inactiveEffects.find(instance);
			if (old != inactiveEffects.end())
			{
				activeEffects[instance] = old->second;
				inactiveEffects.erase(old);
				i = activeEffects.find(instance);
			}
			else i = activeEffects.end();
		}
		if (i != activeEffects.end())
		{
			i->second.deleteObject = bindingEnabled;
			i->second.id = id;
			i->second.enabled = bindingEnabled;
			if (id) effectIDs[id] = i->second.object;
			return i->second.object;
		}
		instance->Reset();
		EntityEffectBinding& binding = activeEffects[instance];
		BYTE instanceOwnership = own;
		binding.object = new VisualEffect(entityID, instance, instanceOwnership ? 0 : release, 0);
		binding.deleteObject = bindingEnabled;
		binding.elapsedSeconds = 0;
		binding.instance = instance;
		binding.releaseInstance = instanceOwnership;
		binding.id = id;
		binding.enabled = bindingEnabled;
		if (id) effectIDs[id] = binding.object;
		return binding.object;
	}

	void Entity::ClearVisualEffects()
	{
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = activeEffects.begin();
			i != activeEffects.end(); ++i)
		{
			if (i->second.object && i->second.deleteObject) delete i->second.object;
			if (i->second.instance && i->second.releaseInstance)
				GetVfxVegCache().Release(i->second.instance);
		}
		activeEffects.clear();
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = inactiveEffects.begin();
			i != inactiveEffects.end(); ++i)
		{
			if (i->second.object && i->second.deleteObject) delete i->second.object;
			if (i->second.instance && i->second.releaseInstance)
				GetVfxVegCache().Release(i->second.instance);
		}
		inactiveEffects.clear();
		effectIDs.clear();
	}

	void Entity::AddInventoryResource(const std::string& name)
	{
		DWORD type = GetWorldInstance()->ClassifyEntity(name, 0);
		Entity* item = GetWorldInstance()->NewEntity(type);
		item->Create(name.c_str(), 0, 0);
		inventory->Add(item);
	}

	void Entity::ApplyAssociationSelection(BYTE value)
	{
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = activeEffects.begin();
			i != activeEffects.end(); ++i)
			if (i->second.object) i->second.object->selectable = value != 0;
	}

	BYTE Entity::SeparateTextures() const { return 0; }

	void Entity::SetSelectable(BYTE value)
	{
		selectable = value;
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = activeEffects.begin(); i != activeEffects.end(); ++i)
			if (i->second.object) i->second.object->selectable = value != 0;
	}

	void Entity::SetShadow(BYTE value)
	{
		shadow = value;
		if (model) model->SetShadow(value);
	}

	BYTE Entity::ReadResource(const char* name, BYTE skipDefaultOverride)
	{
		BYTE result = 0;
		Resources::ResourceMemoryStream* stream = Resources::g_pResourceSystem->OpenMemoryStream(name, 1);
		if (stream)
		{
			result = ReadChunk(stream, skipDefaultOverride);
			if (result == 1) Resources::g_pResourceSystem->CloseMemoryStream(stream);
		}
		return result;
	}

	BYTE Entity::ReadChunk(Resources::ResourceMemoryStream* stream, BYTE skipDefaultOverride)
	{
		BYTE result = 1;
		Common::Chunk chunk;
		if (chunk.Read(stream))
		{
			for (;;)
			{
				if (!ReadFromFile(chunk, stream, skipDefaultOverride))
				{
					LogDebugString("Error reading chunk from entity file.\n");
					result = 0;
					break;
				}
				chunk.SkipToEnd();
				if (!chunk.Read(stream)) break;
			}
		}
		SetSelectable(selectable);
		return result;
	}

	BYTE Entity::ReadFromFile(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		BYTE result;
		switch (chunk.type)
		{
		case '2NEE': result = ReadEEN2(chunk, stream, skipDefaultOverride); break;
		case 'TNEE': result = ReadEENT(chunk, stream); break;
		default: return 1; // Actual base dispatch ignores unrelated/derived chunks.
		}
		if (!result)
		{
			LogDebugString("Error reading entity engine data from entity file.");
			Resources::g_pResourceSystem->CloseMemoryStream(stream);
			return 0;
		}
		return 1;
	}

	BYTE Entity::ReadEENT(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream)
	{
		LogDebugString("Warning: reading obsolete entity chunk from creature file.\n");
		DWORD version = chunk.version;
		DWORD count, word;
		BYTE value, effectCount;
		if (!Common::ReadString(modelName, stream) || !Common::ReadString(scriptName, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&count, 4, stream)) return 0;
		for (DWORD variable = 0; variable < count; ++variable)
		{
			std::string name;
			if (!Common::ReadString(name, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&word, 4, stream)) return 0;
			variables[name] = word;
		}
		if (!Common::ReadString(roleName, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		flag296 = value != 0;
		if (!Common::ReadString(displayName, stream) || !Common::ReadString(textureOverride, stream)
			|| !Common::ReadString(inventoryTexture, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		enabled = value != 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&effectCount, 1, stream)) return 0;
		for (BYTE effect = 0; effect < effectCount; ++effect)
		{
			std::string name;
			if (!Common::ReadString(name, stream)) return 0;
			const char* effectName = name.c_str();
			DWORD id = static_cast<Server*>(F3::g_pNetworkInstance->server)->AllocateVisualEffectID();
			AttachVisualEffect(effectName, id, 1);
		}
		if (!Resources::g_pResourceSystem->ReadMemory(&value17C, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&value36C, 4, stream)) return 0;
		if (version >= 1)
		{
			if (!Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
			selectable = value != 0;
		}
		return 1;
	}

	BYTE Entity::ReadEEN2(Common::Chunk&, Resources::ResourceMemoryStream* stream, BYTE skipDefaultOverride)
	{
		BYTE value, unused[3];
		if (!Common::ReadString(modelName, stream) || !Common::ReadString(inventoryTexture, stream)
			|| !Common::ReadString(activeTexture, stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		flag296 = value != 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		selectable = value != 0;
		for (DWORD i = 0; i < 3; ++i)
			if (!Resources::g_pResourceSystem->ReadMemory(unused + i, 1, stream)) return 0;
		Common::Chunk overrides;
		if (!overrides.Read(stream) || (!skipDefaultOverride && !ReadEEOV(overrides, stream))) return 0;
		overrides.SkipToEnd();
		return 1;
	}

	BYTE Entity::ReadEEOV(Common::Chunk& chunk, Resources::ResourceMemoryStream* stream)
	{
		if (chunk.type != 'VOEE') return 0;
		const DWORD version = chunk.version;
		BYTE discarded[3], shadowValue, effectCount, enabledValue, flagValue;
		DWORD variableCount, word, ignoredWord;
		if (!Common::ReadString(displayName, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(discarded, 1, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(discarded + 1, 1, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(discarded + 2, 1, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&value17C, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&value36C, 4, stream)
			|| !Common::ReadString(roleName, stream)
			|| !Common::ReadString(scriptName, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&variableCount, 4, stream)) return 0;
		for (DWORD i = 0; i < variableCount; ++i)
		{
			std::string name;
			if (!Common::ReadString(name, stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&word, 4, stream)) return 0;
			variables[name] = word;
		}
		if (!Resources::g_pResourceSystem->ReadMemory(&shadowValue, 1, stream)) return 0;
		SetShadow(shadowValue != 0);
		if (!Resources::g_pResourceSystem->ReadMemory(&ignoredWord, 4, stream)
			|| !Common::ReadString(textureOverride, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&effectCount, 1, stream)) return 0;
		for (BYTE i = 0; i < effectCount; ++i)
		{
			std::string effect;
			if (!Common::ReadString(effect, stream)) return 0;
			const char* effectName = effect.c_str();
			DWORD id = static_cast<Server*>(F3::g_pNetworkInstance->server)->AllocateVisualEffectID();
			AttachVisualEffect(effectName, id, 1);
		}
		if (version >= 1)
		{
			if (!Resources::g_pResourceSystem->ReadMemory(&enabledValue, 1, stream)) return 0;
			enabled = enabledValue != 0;
			if (!Resources::g_pResourceSystem->ReadMemory(&flagValue, 1, stream)) return 0;
			flag17A = flagValue != 0;
		}
		if (version >= 2)
		{
			if (!Resources::g_pResourceSystem->ReadMemory(&word, 4, stream)) return 0;
			for (DWORD i = 0; i < word; ++i)
			{
				std::string name;
				if (!Common::ReadString(name, stream)) return 0;
				DWORD type = GetWorldInstance()->ClassifyEntity(name, 0);
				Entity* item = GetWorldInstance()->NewEntity(type);
				item->Create(name.c_str(), 0, 0);
				inventory->Add(item);
			}
		}
		SetSelectable(selectable);
		return 1;
	}

	ModelInstance* Entity::AcquireModel(BYTE loadAnimations)
	{
		ModelInstance* instance = NULL;
		if (!modelName.empty())
		{
			instance = g_pModelCache->AcquireModelInstance(modelName,
				SeparateTextures(), textureOverride.c_str(), loadAnimations);
			if (instance) instance->SetShadow(shadow);
		}
		return instance;
	}

	void Entity::ReleaseModel()
	{
		if (model) { g_pModelCache->ReleaseModelInstance(model); model = NULL; }
	}

	void Entity::SetModel(const char* name, BYTE loadAnimations)
	{
		modelName = name;
		std::string::size_type extension = modelName.find_last_of('.');
		if (extension != std::string::npos)
		{
			std::string root = modelName.substr(0, extension);
			modelName = root;
		}
		ReleaseModel();
		model = AcquireModel(loadAnimations);
		if (model)
		{
			model->GetBounds(modelMinimum, modelMaximum);
			minimum = modelMinimum; maximum = modelMaximum;
			model->SetPosition(position.x, position.y, position.z);
			model->SetRotation(rotation);
		}
		else
		{
			minimum.x = minimum.y = minimum.z = 0;
			modelMinimum.x = modelMinimum.y = modelMinimum.z = 0;
			modelMaximum.x = modelMaximum.y = modelMaximum.z = 0;
			maximum = modelMaximum;
		}
	}

	const char* Entity::GetTextureOverrideName() const
	{
		return textureOverride.c_str();
	}
	void Entity::RefreshTexture()
	{
		if (model) model->SetSocketModel(0, textureOverride.c_str());
	}

	void Entity::SetRotation(const D3DXVECTOR3& value)
	{
		memcpy(&rotation, &value, sizeof(rotation));
		if (model)
		{
			model->SetRotation(value);
			RotateBounds();
			if (mapHandle)
			{
				IWorld* world = GetWorldInstance();
				Map* owner = world->GetMap(mapHandle);
				if (owner) AttachToOctree(owner->GetOctree());
			}
		}
	}

	void Entity::RemoveAttachment(EffectTreeTrigger* trigger)
	{
		lights.remove(trigger);
	}

	void Entity::MoveLight(EntityLight* light, const D3DXVECTOR3& value)
	{
		light->SetPosition(value);
	}

	void Entity::SetPosition(const D3DXVECTOR3& value)
	{
		ChangePosition(value);
		Map* owner = NULL;
		if (mapHandle)
		{
			IWorld* world = GetWorldInstance();
			owner = world->GetMap(mapHandle);
		}
		if (model) model->SetPosition(position.x, position.y, position.z);
		for (std::list<EntityLight*>::iterator i = lights.begin(); i != lights.end(); ++i) MoveLight(*i, value);
		if (spatialFootprint)
		{
			if (!owner) { UpdateVisibility(0); return; }
			if (F3::g_pNetworkInstance->GetMode() & 1)
			{
				PathPoint converted;
				PathPoint* cell = WorldToPathCell(&converted, value, owner);
				LONG x = cell->x, row = cell->y;
				float angle = rotation.x; //54A95E captures once across both virtual queries.
				BYTE positionMatches = spatialFootprint->Matches(x,row,angle);
				BYTE angleMatches = spatialFootprint->MatchesAngle(angle);
				if (!positionMatches || !angleMatches)
				{
					RemoveFootprint(owner->entityGrid);
					if (!angleMatches)
					{
						if (spatialFootprint) delete spatialFootprint;
						spatialFootprint = CreateSpatialFootprint();
					}
					spatialFootprint->Move(x,row);
					AddFootprint(owner->entityGrid);
				}
			}
		}
		if (owner) owner->NotifyEntityMoved(this);
		UpdateVisibility(0);
	}

	void Entity::SnapToFloor()
	{
		D3DXVECTOR3 origin(position.x, float(double(maximum.y)-minimum.y+position.y), position.z);
		IWorld* world = GetWorldInstance();
		Map* owner = world->GetMap(mapHandle);
		float height;
		if (owner->GetOctree()->QueryFloorHeight(&origin.x,&height,1,0,NULL) == 1)
		{
			origin.y = height;
			D3DXVECTOR3 grounded = origin;
			SetPosition(grounded);
		}
	}

	void Entity::GetSpatialCenter(D3DXVECTOR3& output) const
	{
		output.x = 0.0f;
		output.y = (maximum.y - minimum.y) * 0.5f;
		output.z = 0.0f;
	}

	DWORD Entity::GetContextAction(Entity*) { return 0; } //404BB0.
	void Entity::GetContextActions(Entity*, std::list<ContextMenuEntry>&) {} //408090.
	void Entity::UnresolvedSlot38(DWORD,DWORD) {} //Original+38 empty RET8; argument types/count unresolved.
	BYTE Entity::IsSelectionDisabled() const { return selectable ? 0 : 1; } //Original+84,4080A0.
	std::string Entity::GetDisplayName() { return ""; } //Original+A8,5505B0.
	BYTE Entity::CanTalk() const { return dialogueEnabled && !roleName.empty(); } //408400.

	void Entity::RotateBounds()
	{
		D3DXMATRIX matrix;
		D3DXMatrixRotationYawPitchRoll(&matrix, rotation.x * 0.017453292f,
			rotation.y * 0.017453292f, rotation.z * 0.017453292f);
		D3DXVECTOR3 corners[8] =
		{
			D3DXVECTOR3(modelMinimum.x,modelMinimum.y,modelMinimum.z),
			D3DXVECTOR3(modelMaximum.x,modelMinimum.y,modelMinimum.z),
			D3DXVECTOR3(modelMaximum.x,modelMinimum.y,modelMaximum.z),
			D3DXVECTOR3(modelMinimum.x,modelMinimum.y,modelMaximum.z),
			D3DXVECTOR3(modelMinimum.x,modelMaximum.y,modelMinimum.z),
			D3DXVECTOR3(modelMaximum.x,modelMaximum.y,modelMinimum.z),
			D3DXVECTOR3(modelMaximum.x,modelMaximum.y,modelMaximum.z),
			D3DXVECTOR3(modelMinimum.x,modelMaximum.y,modelMaximum.z)
		};
		D3DXVECTOR3 rotatedMinimum(FLT_MAX,FLT_MAX,FLT_MAX);
		D3DXVECTOR3 rotatedMaximum(-FLT_MAX,-FLT_MAX,-FLT_MAX);
		for (DWORD i = 0; i < 8; ++i)
		{
			D3DXVECTOR3 point;
			D3DXVec3TransformCoord(&point, &corners[i], &matrix);
			if (point.x < rotatedMinimum.x) rotatedMinimum.x = point.x;
			if (point.y < rotatedMinimum.y) rotatedMinimum.y = point.y;
			if (point.z < rotatedMinimum.z) rotatedMinimum.z = point.z;
			if (point.x > rotatedMaximum.x) rotatedMaximum.x = point.x;
			if (point.y > rotatedMaximum.y) rotatedMaximum.y = point.y;
			if (point.z > rotatedMaximum.z) rotatedMaximum.z = point.z;
		}
		minimum = rotatedMinimum;
		maximum = rotatedMaximum;
	}
}
