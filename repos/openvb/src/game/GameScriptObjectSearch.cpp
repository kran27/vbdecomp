#include "GameScriptNatives.hpp"
#include "GameScriptObjectSearch.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "entity/Trigger.hpp"
#include "graphics/Octree.hpp"
#include "graphics/OctreeShape.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

namespace
{
	const D3DXVECTOR3* g_ObjectSearchPosition; //Original70BEB0, borrowed through sort.
	BOOL __cdecl CompareObjectDistance(void* first, void* second)
	{
		const D3DXVECTOR3* origin=g_ObjectSearchPosition;
		const JE::Entity* firstEntity=static_cast<JE::Entity*>(first);
		const JE::Entity* secondEntity=static_cast<JE::Entity*>(second);
		double firstX=double(origin->x)-firstEntity->position.x;
		double firstY=double(origin->y)-firstEntity->position.y;
		double firstZ=double(origin->z)-firstEntity->position.z;
		D3DXVECTOR3 secondDelta(origin->x-secondEntity->position.x,
			origin->y-secondEntity->position.y,origin->z-secondEntity->position.z);
		double firstDistance=std::sqrt(firstZ*firstZ+firstY*firstY+firstX*firstX);
		return firstDistance>=std::sqrt(double(secondDelta.z*secondDelta.z
			+secondDelta.y*secondDelta.y+secondDelta.x*secondDelta.x));
	}
	DWORD FindObjects(DWORD* handle,DWORD mapID,const D3DXVECTOR3& position,float radius,DWORD exclude)
	{
		JE::Map* map=JE::g_World->GetMap(mapID);
		OCTREE* tree=map->GetOctree();
		OctreeShapeConfiguration configuration; //Original value0 remains unwritten/type1 does not read it.
		configuration.radius=radius;
		configuration.type=1;
		OCTREE_Shape* shape;
		if (tree->CreateShape(&configuration,&shape))
		{
			JE::LogDebugString("ObjectLists::FindObjects() Error: OCTREE::CreateShape() Failed.");
			return 0;
		}
		D3DXVECTOR3 shapePosition(position);
		if (shape->SetPosition(&shapePosition.x))
		{
			JE::LogDebugString("ObjectLists::FindObjects() Error: OCTREE_Shape::Move() Failed.");
			return 0; //Original leaves this allocated shape behind.
		}
		if (tree->UpdateShape(shape))
		{
			JE::LogDebugString("ObjectLists::FindObjects() Error: OCTREE::ApplyShape() Failed.");
			return 0; //No invented rollback/Release on the original failure prefix.
		}
		F3::FrameLookupCaches::ObjectList* results=new F3::FrameLookupCaches::ObjectList;
		void* objects[32];
		DWORD more;
		if (!shape->QueryObjects(32,objects,&more))
		{
			do
			{
				for (DWORD i=0;i<32;++i)
				{
					if (!objects[i]) break;
					JE::Entity* entity=dynamic_cast<JE::Entity*>(static_cast<JE::DynamicObject*>(objects[i]));
					if (entity && entity->entityID!=exclude && entity->renderEnabled)
						results->push_back(entity);
				}
			} while (more && !shape->QueryObjects(32,objects,&more));
		}
		tree->InvalidateShape(shape);
		shape->Release();
		DWORD count=F3::PublishScriptObjectList(handle,results);
		if (count)
		{
			g_ObjectSearchPosition=&position;
			std::sort(results->begin(),results->end(),CompareObjectDistance);
			return DWORD(results->size());
		}
		return 0;
	}
}

namespace F3
{
	DWORD FindScriptObjects(DWORD* handle,DWORD mapID,const D3DXVECTOR3& position,float radius,DWORD exclude)
	{
		return FindObjects(handle,mapID,position,radius,exclude);
	}

	cell AMX_NATIVE_CALL NativeFindObjects(AMX* vm,cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,3,"FindObjects")) return 0;
		D3DXVECTOR3 position(0,0,0);
		if (!JE::ScriptServices::ReadNativeVector(vm,parameters[2],position))
		{
			LogScriptError(vm,"FindObjects() was unable to retreive the position.");
			return 0;
		}
		float radius;
		if (!JE::ScriptServices::ReadNativeFloat(vm,parameters[3],radius))
		{
			LogScriptError(vm,"FindObjects() was unable to retrieve the radius.");
			return 0;
		}
		void* ownerData;
		if (amx_GetUserData(vm,0x464C534D,&ownerData))
		{
			LogScriptError(vm,"FindObjects() was unable to retrieve 'myself' id.");
			return 0;
		}
		DWORD owner=reinterpret_cast<DWORD>(ownerData),mapID=owner,exclude=0;
		switch (owner>>29)
		{
		case 0:
			{
				exclude=owner;
				JE::Entity* entity=JE::GetEntityPtr(owner);
				mapID=entity ? entity->mapHandle : 0;
			}
			break;
		case 1: break; //The owner itself is the map ID.
		case 2:
			{
				JE::Trigger* trigger=JE::FindTrigger(owner);
				mapID=trigger ? trigger->mapID : 0;
			}
			break;
		default: mapID=0; break;
		}
		if (!mapID)
		{
			LogScriptError(vm,"FindObjects() was unable to resolve the map.");
			return 0;
		}
		DWORD handle;
		DWORD count=FindObjects(&handle,mapID,position,radius,exclude);
		if (count)
		{
			cell* destination;
			if (amx_GetAddr(vm,parameters[1],&destination))
			{
				LogScriptError(vm,"FindObjects() was unable to resolve list handle address.");
				return 0; //Original already published the cache; it is not rolled back.
			}
			*destination=cell(handle);
		}
		return cell(count); //Zero matches leave the output cell unchanged.
	}
}
