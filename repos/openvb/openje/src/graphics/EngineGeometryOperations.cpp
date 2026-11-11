#include "Engine.hpp"
#include "ModelDataManager.hpp"
#include "BufferManager.hpp"
#include "MeshData.hpp"
#include "Viewport.hpp"
#include "RenderManager.hpp"

DWORD Gfx_Engine::RegisterModelData(Gfx_ModelData526620* object)
{
	return g_ModelDataManager->Register(object);
}

DWORD Gfx_Engine::ModelDataReferences(DWORD* handle)
{
	return g_ModelDataManager->References(handle);
}

DWORD Gfx_Engine::ReleaseModelData(DWORD* handle)
{
	return g_ModelDataManager->Release(handle);
}

Gfx_ModelInstance* Gfx_Engine::CreateModelInstance(DWORD handle)
{
	return g_ModelDataManager->CreateInstance(handle);
}

bool Gfx_Engine::CreateGeometryData(Gfx_MeshData* data)
{
	return g_BufferManager->CreateStoredBuffers(data);
}

void Gfx_Engine::ReleaseGeometryData(Gfx_MeshData* data)
{
	DWORD handle = data->value04;
	g_BufferManager->ReleaseStoredHandle(handle);
}

void Gfx_Engine::QueueRenderable(void* renderable, Gfx_Viewport* viewport,
	BYTE flag, DWORD value8, DWORD valueC)
{
	if (viewport)
	{
		viewport->renderer->QueueSkinMesh(static_cast<Gfx_SkinMeshPkt*>(renderable),
			flag, value8, valueC);
	}
	else
	{
		defaultViewport->renderer->QueueSkinMesh(static_cast<Gfx_SkinMeshPkt*>(renderable),
			flag, value8, valueC);
	}
}
