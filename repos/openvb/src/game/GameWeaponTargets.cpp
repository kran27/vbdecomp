#include "GameWeapon.hpp"
#include "GameActionAttack.hpp"
#include "GameCreature.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/Octree.hpp"
#include "graphics/OctreeShape.hpp"
#include "graphics/GfxVector3.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <float.h>
#include <math.h>
#include <vector>

#include <hash_map>

namespace F3
{
	namespace
	{
		struct AngularHash
		{
			enum { bucket_size=4,min_buckets=8 };
			size_t operator()(float value) const { return static_cast<size_t>(value); }
			bool operator()(float first,float second) const { return first<second; }
		};
		typedef std::hash_multimap<float,GameCreature*,AngularHash> AngularCandidates;
		float DirectionYaw(const D3DXVECTOR3& origin,const D3DXVECTOR3& target)
		{
			Gfx_Vector3f direction;
			direction.value[0]=target.x-origin.x;
			direction.value[1]=target.y-origin.y;
			direction.value[2]=target.z-origin.z;
			direction.NormalizeDirection();
			float dot=direction.value[1]*0.0f;
			dot+=direction.value[0]*0.0f;
			dot+=direction.value[2];
			float angle=float(acos(dot));
			if (direction.value[0]<0.0f) angle=-angle;
			return angle;
		}
		void TransformShape(OCTREE_Shape* shape,const D3DXVECTOR3& origin,float radians)
		{
			D3DXMATRIX translation,rotation,transform;
			D3DXMatrixTranslation(&translation,origin.x,origin.y,origin.z); //4722AD.
			D3DXMatrixRotationY(&rotation,radians); //4723C7.
			D3DXMatrixMultiply(&transform,&rotation,&translation); //471950.
			shape->SetTransform(&transform._11);
		}
		GameCreature* QueryCreature(void* object)
		{
			return dynamic_cast<GameCreature*>(static_cast<JE::DynamicObject*>(object));
		}
		void CollectCone(GameWeapon* weapon,OCTREE* tree,JE::Entity* requested,
			const D3DXVECTOR3& origin,const AttackSubAttack& sub,DWORD range,
			std::vector<GameCreature*>& candidates)
		{
			OctreeShapeConfiguration configuration;
			configuration.first=45.0f;
			configuration.radius=float(range);
			configuration.type=2;
			OCTREE_Shape* shape;
			tree->CreateShape(&configuration,&shape); //415890/416394 ignore result.
			if (!shape) return;
			TransformShape(shape,origin,float(double(sub.rotation.x)*0.01745329238474369f));
			tree->UpdateShape(shape);
			DWORD more=1;
			void* objects[50];
			do
			{
				shape->QueryObjects(50,objects,&more);
				for (DWORD i=0;i<50 && objects[i];++i) //53B290 reserves the last slot for NULL.
				{
					GameCreature* creature=QueryCreature(objects[i]);
					if (creature && static_cast<JE::Entity*>(creature)!=requested
						&& !creature->IsDead() && weapon->GetLOS(creature))
						candidates.push_back(creature);
				}
			} while (more);
			tree->InvalidateShape(shape);
			shape->Release();
		}
		BYTE CandidateUnobstructed(OCTREE* tree,JE::Entity* requested,
			GameCreature* candidate,const D3DXVECTOR3& origin,float* yaw)
		{
			D3DXVECTOR3 target=candidate->position;
			Gfx_Vector3f originValue={{origin.x,origin.y,origin.z}};
			Gfx_Vector3f targetValue={{target.x,target.y,target.z}};
			float distance=float(originValue.Distance(targetValue.value));
			OctreeShapeConfiguration configuration;
			configuration.first=0;
			configuration.radius=distance;
			configuration.type=4;
			OCTREE_Shape* shape;
			tree->CreateShape(&configuration,&shape); //415B8A/41667A.
			if (!shape) return 0; //Original skips this candidate on a NULL shape.
			*yaw=DirectionYaw(origin,target);
			TransformShape(shape,origin,*yaw);
			tree->UpdateShape(shape);
			DWORD more=1;
			BYTE clear=1;
			void* objects[50];
			do
			{
				shape->QueryObjects(50,objects,&more);
				for (DWORD i=0;i<50 && objects[i];++i)
				{
					GameCreature* other=QueryCreature(objects[i]);
					if (other && other!=candidate && static_cast<JE::Entity*>(other)!=requested
						&& !other->IsDead())
					{
						Gfx_Vector3f otherPosition={{other->position.x,other->position.y,other->position.z}};
						if (originValue.Distance(otherPosition.value)<distance)
						{
							clear=0;
							more=0;
							break;
						}
					}
				}
			} while (more);
			tree->InvalidateShape(shape);
			shape->Release();
			return clear;
		}
	}

	BYTE GameWeapon::ResolveTargets(JE::Entity* attacker, JE::Entity* requested,
		AttackModeIterator attack, const AttackSubAttack& sub, BYTE pointTarget,
		const D3DXVECTOR3& point, std::list<JE::Entity*>& output)
	{
		//415740 validates these three actual parameters; it does not clear output.
		//The point argument is passed by the parent but these actual branches use
		//the authored subattack rotation and live creature positions.
		(void)point;
		if (!attacker || (!pointTarget && !requested) || attack == AttackModeIterator())
		{
			JE::LogDebugString("GameWeapon::ResolveTargets() - NULL parameters recieved.\n");
			return 0;
		}
		switch (attack->animation)
		{
		case 0: case 2: //4169BA.
		case 6: case 7: case 8: case 9: //4157B0/41699E.
			if (requested) output.push_back(requested); //Actual pointer append402B30; duplicates retained.
			return 1;
		case 1: case 4:
			{
				//4157E2/415F75/4162E9. Use the actual registry/map/octree/model
				//owners before the original spatial-query child.
				DWORD mapID=attacker->mapHandle;
				if (!mapID) return 0;
				JE::IWorld* world=JE::GetWorldInstance();
				JE::Map* map=world->GetMap(mapID);
				if (!map) return 0;
				OCTREE* tree=map->GetOctree();
				if (!tree || !model) return 0;
				if (attack->animation==4 || sub.mode->flag66)
				{
					D3DXVECTOR3 origin;
					model->GetAttachedPosition(origin); //563A40, includes real held-bone transform.
					std::vector<GameCreature*> candidates;
					CollectCone(this,tree,requested,origin,sub,attack->range,candidates);
					if (attack->animation==4)
					{
						for (size_t i=0;i<candidates.size();++i)
						{
							float yaw;
							if (CandidateUnobstructed(tree,requested,candidates[i],origin,&yaw))
								output.push_back(candidates[i]);
						}
						return 1;
					}
					AngularCandidates angular;
					for (size_t i=0;i<candidates.size();++i)
					{
						float yaw;
						if (CandidateUnobstructed(tree,requested,candidates[i],origin,&yaw))
						{
							//415C27 subtracts the stored degree value directly from the
							//radian yaw. Preserve the original mixed-unit priority key.
							float key=float(fabs(double(yaw)-sub.rotation.x));
							angular.insert(AngularCandidates::value_type(key,candidates[i]));
						}
					}
					candidates.clear();
					for (AngularCandidates::iterator i=angular.begin();i!=angular.end();++i)
						candidates.push_back(i->second);
					LONG shots=static_cast<LONG>(attack->shots);
					LONG count=static_cast<LONG>(candidates.size());
					for (LONG i=0;i<shots;++i) output.push_back(candidates[i%count]);
					return 1;
				}
				//415F75..4162E4, a narrow zero-width rectangle along this shot.
				OctreeShapeConfiguration configuration;
				configuration.first=0;
				configuration.radius=float(attack->range);
				configuration.type=4;
				OCTREE_Shape* shape;
				tree->CreateShape(&configuration,&shape);
				if (!shape) return 1;
				D3DXVECTOR3 origin;
				model->GetAttachedPosition(origin);
				TransformShape(shape,origin,float(double(sub.rotation.x)*0.01745329238474369f));
				tree->UpdateShape(shape);
				DWORD more=1;
				GameCreature* nearest=NULL;
				float nearestSquared=FLT_MAX;
				void* objects[50];
				do
				{
					shape->QueryObjects(50,objects,&more);
					for (DWORD i=0;i<50 && objects[i];++i)
					{
						GameCreature* creature=QueryCreature(objects[i]);
						if (creature && static_cast<JE::Entity*>(creature)!=requested
							&& !creature->IsDead() && GetLOS(creature))
						{
							Gfx_Vector3f originValue={{origin.x,origin.y,origin.z}};
							Gfx_Vector3f creaturePosition={{creature->position.x,creature->position.y,creature->position.z}};
							double distance=originValue.SquaredDistance(creaturePosition.value);
							if (distance<nearestSquared) { nearestSquared=float(distance); nearest=creature; }
						}
					}
				} while (more);
				//Raw4162BB reads[ESP+460] after one push: the actual seventh
				//argument at[bodyESP+45C], not IDA's invented eighth argument.
				if (nearest) output.push_back(nearest);
				tree->InvalidateShape(shape);
				shape->Release();
				return 1;
			}
		default:
			if (attack->animation>=13 && attack->animation<=34)
				JE::LogDebugString("GameWeapon::ResolveTargets() - Recieved an Unarmed attack type. Error.\n");
			else
				JE::LogDebugString("GameWeapon::ResolveTargets() - Recieved unknown attack mode type: %d\n",attack->kind);
			return 0;
		}
	}
}
