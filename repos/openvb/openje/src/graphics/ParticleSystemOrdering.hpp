#pragma once

#include "ParticleSystem.hpp"

// One shared pair of original scratch matrices,709970/7099B0. Draw publishes
// the camera after geometry; depth ordering deliberately uses that prior value.
namespace GfxParticleDraw
{
	extern D3DXMATRIX particleCamera;
	extern D3DXMATRIX particleTransform;
}

// These declarations name the same objects; they allocate no aliases or copies.
using GfxParticleDraw::particleCamera;
using GfxParticleDraw::particleTransform;

// Original5130C0: native default list::merge calls the particle value ordering.
static bool operator<(const Gfx_ParticleInfo& left, const Gfx_ParticleInfo& right)
{
	D3DXVECTOR3 l(left.position.x, left.position.y, left.position.z);
	D3DXVECTOR3 r(right.position.x, right.position.y, right.position.z);
	D3DXVec3TransformCoord(&l, &l, &particleTransform);
	D3DXVec3TransformCoord(&l, &l, &particleCamera);
	D3DXVec3TransformCoord(&r, &r, &particleTransform);
	D3DXVec3TransformCoord(&r, &r, &particleCamera);
	// The original merge moves the incoming node before the destination
	// when incoming.z > destination.z: descending camera depth.
	return l.z > r.z;
}
