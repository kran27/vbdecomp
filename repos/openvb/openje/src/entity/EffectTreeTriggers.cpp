#include <winsock2.h>
#include "EffectTreeTriggers.hpp"
#include "Map.hpp"
#include "World.hpp"
#include "graphics/Octree.hpp"
#include "graphics/OctreeShape.hpp"
#include <cmath>
#include <stdexcept>

namespace JE
{
	EffectTreeCircleTrigger::~EffectTreeCircleTrigger() {}
	EffectTreeCircleTrigger::EffectTreeCircleTrigger() : radius(0)
	{
		forceShapeMembership = 1;
	}
	BYTE EffectTreeCircleTrigger::Contains(const D3DXVECTOR3& point) const
	{
		const float x = position.x - point.x; //5B236A stores this component as float.
		const double z = double(position.z) - point.z;
		return std::sqrt(z * z + double(x) * x) <= radius;
	}
	BYTE EffectTreeCircleTrigger::CreateShape()
	{
		if (!mapID) return 0;
		IWorld* world = g_World;
		Map* map = world->GetMap(mapID);
		if (!map) return 0;
		OctreeShapeConfiguration configuration;
		configuration.radius = radius;
		configuration.type = 1; //First DWORD remains unwritten and is unused by Circle.
		map->GetOctree()->CreateShape(&configuration, &shape);
		return shape != NULL;
	}

	EffectTreeSemiCircleTrigger::~EffectTreeSemiCircleTrigger() {}
	EffectTreeSemiCircleTrigger::EffectTreeSemiCircleTrigger() : radius(0)
	{
		forceShapeMembership = 1; //angle remains unwritten, as in5B20D0.
	}
	BYTE EffectTreeSemiCircleTrigger::Contains(const D3DXVECTOR3& point) const
	{
		const double x = double(point.x) - position.x;
		const double z = double(point.z) - position.z;
		if (std::sqrt(z * z + x * x) > radius) return 0;
		const double radians = double(angle) * 0.01745329238474369f; //649C50, float bits3C8EFA35.
		const float directionX = float(std::cos(radians));
		const float directionZ = float(std::sin(radians));
		const double inverseLength = 1.0 / std::sqrt(z * z + x * x);
		const float normalX = float(x * inverseLength);
		const float normalY = float(0.0 * inverseLength);
		const float normalZ = float(z * inverseLength);
		const double dot = double(normalZ) * directionZ + double(normalX) * directionX + double(normalY) * 0.0;
		//5B227C tests AH41/JZ: unordered acos results are accepted too.
		return !(std::acos(dot) > 0.31830987334251404f); //6A9DB4, float bits3EA2F983.
	}
	BYTE EffectTreeSemiCircleTrigger::CreateShape()
	{
		if (!mapID) return 0;
		IWorld* world = g_World;
		Map* map = world->GetMap(mapID);
		if (!map) return 0;
		OctreeShapeConfiguration configuration;
		configuration.first = 180.0f;
		configuration.radius = radius;
		configuration.type = 2;
		map->GetOctree()->CreateShape(&configuration, &shape);
		if (!shape) return 0;
		D3DXMATRIX transform;
		D3DXMatrixIdentity(&transform);
		//The shape uses the raw angle as radians here; Contains converts it first.
		transform._11 = float(std::cos(double(angle)));
		transform._31 = float(std::sin(double(angle)));
		shape->SetTransform(reinterpret_cast<const float*>(&transform));
		return 1;
	}

	EffectTreeRingTrigger::~EffectTreeRingTrigger() {}
	EffectTreeRingTrigger::EffectTreeRingTrigger() : innerRadius(0), outerRadius(0)
	{
		forceShapeMembership = 1;
	}
	BYTE EffectTreeRingTrigger::Contains(const D3DXVECTOR3& point) const
	{
		const float x = position.x - point.x; //5B206A stores this component as float.
		const double z = double(position.z) - point.z;
		const double distance = std::sqrt(z * z + double(x) * x);
		return distance >= innerRadius && distance <= outerRadius;
	}
	BYTE EffectTreeRingTrigger::CreateShape()
	{
		if (!mapID) return 0;
		IWorld* world = g_World;
		Map* map = world->GetMap(mapID);
		if (!map) return 0;
		OctreeShapeConfiguration configuration;
		configuration.first = innerRadius;
		configuration.radius = outerRadius;
		configuration.type = 3;
		map->GetOctree()->CreateShape(&configuration, &shape);
		return shape != NULL;
	}
}
