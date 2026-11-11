#include "RenderManager.hpp"
#include "VertexBuffer.hpp"
#include "HeightFog.hpp"
#include "DebugSphere.hpp"
#include "diagnostics/DebugOutput.hpp"

Gfx_RenderManager::Gfx_RenderManager(Gfx_Viewport* viewport)
	: owner(viewport), quadBuffer(0), projectedShadows(0), flag40(0), heightFog(0),
	flag48(0), flag49(0), floor(0), flag74(0)
{
	// This store follows all seven real container constructions in the original.
	// Padding, allocator representation and unrelated child/global state stay unwritten.
	shadowColor = 0x80000000;
}

Gfx_RenderManager::~Gfx_RenderManager()
{
	ReleaseResources();
	// Real map/list members then destroy in reverse declaration order.
}

void Gfx_RenderManager::ReleaseResources()
{
	ReleaseHeightFog();
	if (quadBuffer)
	{
		delete quadBuffer;
		quadBuffer = 0;
	}
	DeleteDebugSpheres();
}

void Gfx_RenderManager::ReleaseHeightFog()
{
	if (heightFog)
	{
		delete heightFog;
		heightFog = 0;
	}
	flag40 = 0;
}

BYTE Gfx_RenderManager::EnableHeightFog(BYTE enabled)
{
	if (heightFog)
	{
		flag40 = enabled;
		return 1;
	}
	if (enabled)
	{
		JE::LogDebugString("Gfx_RenderManager::EnableHeightFog - [WARNING] no height fog created");
	}
	flag40 = 0;
	return 0;
}

void Gfx_RenderManager::DeleteDebugSpheres()
{
	std::list<Gfx_DebugSphere505880*>::iterator entry = usedDebugSpheres.begin();
	while (entry != usedDebugSpheres.end())
	{
		Gfx_DebugSphere505880* sphere = *entry;
		if (sphere)
		{
			delete sphere;
			*entry = 0;
		}
		entry = usedDebugSpheres.erase(entry);
	}
	entry = freeDebugSpheres.begin();
	while (entry != freeDebugSpheres.end())
	{
		Gfx_DebugSphere505880* sphere = *entry;
		if (sphere)
		{
			delete sphere;
			*entry = 0;
		}
		entry = freeDebugSpheres.erase(entry);
	}
}
