#include "PacketManager.hpp"
#include "PacketBuffer.hpp"
#include "PacketStorage.hpp"
#include "Manager.hpp"
#include "graphics/SystemInterface.hpp"
#include <cstring>

namespace GUI
{
	void CPktManager::BatchQuads(std::list<Gfx_MeshInstance*>::iterator& position, LONG count, DWORD textureHandle)
	{
		if (count <= 0) return;
		Gfx_UIPacketBuffer* batch = g_Manager.storage->AcquireBatch();
		batch->ConfigureQuads(static_cast<DWORD>(count), static_cast<LONG>(static_cast<DWORD>(count) * 4), textureHandle);
		entries38.push_back(batch);
		DWORD offset = 0;
		do
		{
			Gfx_MeshInstance* instance = *position;
			if (instance)
			{
				std::memcpy(reinterpret_cast<BYTE*>(batch->vertices) + offset, instance->mesh->vertices, 112);
			}
			offset += 112;
			--count;
			++position;
		}
		while (count);
		Gfx_Viewport* target = reinterpret_cast<Gfx_Viewport*>(g_Manager.view);
		g_GfxSystem->QueueMeshInstances(&batch->instance, target, 1);
	}

	void CPktManager::BatchTriangles(std::list<Gfx_MeshInstance*>::iterator& position, LONG count, LONG primitives,
		DWORD textureHandle, DWORD color)
	{
		if (count <= 0 || primitives <= 0) return;
		Gfx_UIPacketBuffer* batch = g_Manager.storage->AcquireBatch();
		batch->ConfigureTriangles(static_cast<DWORD>(primitives), static_cast<DWORD>(primitives) * 3, textureHandle, color);
		entries38.push_back(batch);
		DWORD vertexOffset = 0;
		do
		{
			Gfx_MeshInstance* instance = *position;
			if (instance)
			{
				DWORD vertices = instance->mesh->vertexCount;
				std::memcpy(reinterpret_cast<BYTE*>(batch->vertices) + vertexOffset * 28,
					instance->mesh->vertices, vertices * 28);
				vertexOffset += vertices;
			}
			++position;
			--count;
		}
		while (count);
		Gfx_Viewport* target = reinterpret_cast<Gfx_Viewport*>(g_Manager.view);
		g_GfxSystem->QueueMeshInstances(&batch->instance, target, 1);
	}
}
