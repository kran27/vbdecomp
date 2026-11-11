#include <winsock2.h>
#include "startup/NetworkComponent.hpp"
#include "EffectTreeTrigger.hpp"
#include "Effect.hpp"
#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "Map.hpp"
#include "World.hpp"
#include "graphics/Octree.hpp"
#include "graphics/OctreeShape.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "model/CommonException.hpp"
#include <algorithm>
#include <cmath>
#include <float.h>
#include <stdexcept>
#include <vector>

namespace JE
{
	namespace
	{
		const D3DXVECTOR3* sortingPosition; //Original70C740, borrowed by5AA450.
		bool FartherFirst(Entity* first, Entity* second)
		{
			double x = double(sortingPosition->x) - first->position.x;
			double y = double(sortingPosition->y) - first->position.y;
			double z = double(sortingPosition->z) - first->position.z;
			float otherX = float(double(sortingPosition->x) - second->position.x);
			float otherY = float(double(sortingPosition->y) - second->position.y);
			float otherZ = float(double(sortingPosition->z) - second->position.z);
			//Original includes equality; no strict-order/tie correction is invented.
			return std::sqrt(double(otherZ)*otherZ + double(otherY)*otherY
				+ double(otherX)*otherX) <= std::sqrt(z*z + y*y + x*x);
		}


	}

	EffectTreeTrigger::EffectTreeTrigger()
		: position(0,0,0), creatorID(0), attachmentOwnerID(0), attachedEntity(NULL),
		reevaluate(1), shape(NULL), forceShapeMembership(0), expirationTime(0), targetLimit(DWORD(-1))
	{
	}

	EffectTreeTrigger::~EffectTreeTrigger()
	{
		if (attachedEntity) attachedEntity->Destroy();
		if (shape)
		{
			shape->Release();
			shape = NULL;
		}
		if (attachmentOwnerID)
		{
			Entity* owner = GetEntityPtr(attachmentOwnerID);
			if (owner) owner->RemoveAttachment(this);
		}
		for (std::map<DWORD,std::list<DWORD> >::iterator entry = appliedEffects.begin();
			entry != appliedEffects.end(); ++entry)
		{
			Entity* entity = GetEntityPtr(entry->first);
			if (entity)
				for (std::list<DWORD>::iterator id = entry->second.begin();
					id != entry->second.end(); ++id) entity->CompleteEffect(*id);
			//Native ignores erase's returned successor and increments the erased iterator.
			appliedEffects.erase(entry);
		}
		while (!templates.empty())
		{
			Effect* effect = templates.front();
			templates.pop_front();
			effect->Destroy();
		}
	}

	BYTE EffectTreeTrigger::Read(Resources::ResourceMemoryStream*) { return 1; }

	void EffectTreeTrigger::SetPosition(const D3DXVECTOR3& value)
	{
		position.x = value.x;
		position.y = value.y;
		position.z = value.z;
		if (attachedEntity) attachedEntity->SetPosition(value);
		polygon = localPolygon;
		polygon.Translate(value);
		reevaluate = 1;
	}

	BYTE EffectTreeTrigger::CreateShape()
	{
		const DWORD selectedMap = mapID;
		if (!selectedMap) return 0;
		IWorld* world = g_World;
		Map* map = world->GetMap(selectedMap);
		if (!map) return 0;
		D3DXVECTOR3 minimum(FLT_MAX,FLT_MAX,FLT_MAX);
		D3DXVECTOR3 current;
		D3DXVECTOR3 maximum(FLT_MIN,FLT_MIN,FLT_MIN); //Original positive minima.
		for (std::vector<D3DXVECTOR3>::iterator point = polygon.points.begin();
			point != polygon.points.end(); ++point)
		{
			current.x = point->x;
			current.y = point->y;
			current.z = point->z;
			if (current.x < minimum.x) minimum.x = current.x;
			if (current.z < minimum.z) minimum.z = current.z;
			if (current.x > maximum.x) maximum.x = current.x;
			if (current.z > maximum.z) maximum.z = current.z;
		}
		OctreeShapeConfiguration configuration;
		configuration.first = maximum.x - minimum.x;
		configuration.radius = maximum.z - minimum.z;
		configuration.type = 4;
		map->GetOctree()->CreateShape(&configuration, &shape);
		return shape != NULL;
	}

	void EffectTreeTrigger::OnEnter(Entity* entity)
	{
		if (appliedEffects.size() == targetLimit) return;
		std::list<DWORD> effectIDs;
		for (std::list<Effect*>::iterator entry = templates.begin(); entry != templates.end(); ++entry)
		{
			Effect* original = *entry;
			if (!(F3::g_pNetworkInstance->GetMode() & 1))
				throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
			IWorldServer* server = g_World;
			Effect* effect = server->NewEffect(original->type);
			effect->CopyFrom(*original);
			effect->SetCreator(creatorID);
			effect->SetTarget(1,0);
			effect->SetTarget(2,entity->entityID);
			effect->SetTarget(0,creatorID);
			effect->SetTarget(3,0);
			effectIDs.push_back(effect->GetID());
			entity->AddEffect(effect);
		}
		const DWORD entityID = entity->entityID; //Native captures the value before map callbacks.
		appliedEffects[entityID] = effectIDs;
	}

	void EffectTreeTrigger::OnLeave(Entity* entity)
	{
		const DWORD entityID = entity->entityID;
		std::map<DWORD,std::list<DWORD> >::iterator entry = appliedEffects.find(entityID);
		if (entry == appliedEffects.end()) return;
		for (std::list<DWORD>::iterator id = entry->second.begin(); id != entry->second.end(); ++id)
			entity->CompleteEffect(*id);
		DWORD oldCount = DWORD(appliedEffects.size());
		appliedEffects.erase(entry);
		if (oldCount == targetLimit)
			for (std::set<DWORD>::iterator id = entities.begin(); id != entities.end(); ++id)
			{
				const DWORD trackedID = *id;
				if (appliedEffects.find(trackedID) == appliedEffects.end())
				{
					OnEnter(GetEntityPtr(trackedID));
					return; //5AA736 refills only the first eligible tracked entity.
				}
			}
	}

	void EffectTreeTrigger::OnUpdate(DWORD time)
	{
		if (time >= expirationTime)
		{
			destroyed = 1;
			return;
		}
		for (std::map<DWORD,std::list<DWORD> >::iterator entry = appliedEffects.begin();
			entry != appliedEffects.end();)
		{
			Entity* entity = GetEntityPtr(entry->first);
			if (entity && entity->mapHandle == mapID) ++entry;
			else entry = appliedEffects.erase(entry); //No CompleteEffect for absent/moved owners.
		}
		if (reevaluate)
		{
			ReevaluateEntities();
			reevaluate = 0;
		}
	}

	void EffectTreeTrigger::ReevaluateEntities()
	{
		std::set<DWORD> previous(entities);
		for (std::set<DWORD>::iterator id = previous.begin(); id != previous.end(); ++id)
			MoveEntity(GetEntityPtr(*id));
		if (!shape && !CreateShape())
		{
			LogDebugString("EffectTreeTrigger::ReevaluateEntities() - Unable to create OCTREE_Shape.\n");
			return;
		}
		const D3DXVECTOR3 selectedPosition = position;
		if (shape->SetPosition(&selectedPosition.x))
		{
			LogDebugString("EffectTreeTrigger::ReevaluateEntities() Error: OCTREE_Shape::Move() Failed.");
			return;
		}
		const DWORD selectedMap = mapID;
		if (!selectedMap) return;
		IWorld* world = g_World;
		Map* map = world->GetMap(selectedMap);
		if (!map) return;
		if (map->GetOctree()->UpdateShape(shape))
		{
			LogDebugString("EffectTreeTrigger::ReevaluateEntities() Error: OCTREE::ApplyShape() Failed.");
			return;
		}
		std::vector<Entity*>* candidates = new std::vector<Entity*>;
		void* objects[32];
		DWORD more;
		int status = shape->QueryObjects(32,objects,&more);
		while (!status)
		{
			for (LONG index = 0; index < 32 && objects[index]; ++index)
			{
				Entity* entity = dynamic_cast<Entity*>(static_cast<DynamicObject*>(objects[index]));
				if (entity && entity->renderEnabled) candidates->push_back(entity);
			}
			if (!more) break;
			status = shape->QueryObjects(32,objects,&more);
		}
		map->GetOctree()->InvalidateShape(shape);
		if (candidates->empty())
		{
			delete candidates;
			return;
		}
		sortingPosition = &position;
		std::sort(candidates->begin(),candidates->end(),FartherFirst);
		if (!forceShapeMembership)
		{
			for (std::vector<Entity*>::iterator entity = candidates->begin(); entity != candidates->end(); ++entity)
				MoveEntity(*entity);
		}
		else
		{
			for (std::vector<Entity*>::iterator entity = candidates->begin(); entity != candidates->end(); ++entity)
			{
				Entity* selected = *entity;
				const DWORD entityID = selected->entityID;
				if (entities.find(entityID) == entities.end())
				{
					TrackEntity(selected);
					OnEnter(selected);
				}
			}
		}
		delete candidates;
	}
}
