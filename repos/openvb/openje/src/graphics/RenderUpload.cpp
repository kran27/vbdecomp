#include "RenderManager.hpp"
#include "BufferManager.hpp"
#include "D3DState.hpp"
#include <cstring>

void Gfx_RenderManager::Upload(UINT pass, BYTE shadowOnly)
{
	for (std::map<DWORD, Gfx_RenderGroup>::iterator staticGroup = staticGroups[0].begin();
		staticGroup != staticGroups[0].end(); ++staticGroup)
	{
		if (staticGroup->second.groupCounts[pass] != 0)
		{
			// Binding failure does not suppress the actual draw child.
			g_BufferManager->BindVertexData(staticGroup->second.mesh->value04);
			DrawGroup(pass, &staticGroup->second, shadowOnly);
		}
	}
	std::list<Gfx_RenderGroup>::iterator group = dynamicGroups[0].begin();
	if (group != dynamicGroups[0].end())
	{
		// Capture once for the dynamic loop; the vertex owner reloads per group.
		Gfx_StreamingIndexBuffer* indexStream = g_BufferManager->indexStream;
		do
		{
			if (group->groupCounts[pass] != 0)
			{
				// Capture the first by-value node, not group.mesh or a later front().
				Gfx_MeshInstance* first = &group->instances.front();
				UINT stride = first->mesh->vertexStride;
				UINT count = first->mesh->vertexCount;
				Gfx_StreamingVertexBuffer* vertexStream = g_BufferManager->vertexStream;
				BYTE* destination = 0;
				UINT vertexBase;
				if (SUCCEEDED(vertexStream->Lock(count, stride, &destination, &vertexBase, 0)))
				{
					std::memcpy(destination, first->mesh->vertices, count * stride);
					vertexStream->buffer->Unlock();
					IDirect3DVertexBuffer8* buffer = vertexStream->buffer;
					++g_D3D.statistics[9];
					g_D3D.device->SetStreamSource(0, buffer, stride);
					// Reload this same first node's mesh after the SDK callback.
					Gfx_MeshData* mesh = first->mesh;
					const void* source = reinterpret_cast<const void*>(mesh->value10);
					if (source != 0)
					{
						UINT indexCount = mesh->value24;
						UINT indexStride = mesh->value20;
						BYTE* indices = 0;
						UINT indexBase;
						if (SUCCEEDED(indexStream->Lock(indexCount, &indices, &indexBase, 0)))
						{
							std::memcpy(indices, source, indexCount * indexStride);
							indexStream->buffer->Unlock();
							group->indexBase = indexBase;
							g_D3D.device->SetIndices(indexStream->buffer, vertexBase);
						}
						// Failure deliberately retains the previous indexBase/binding.
					}
					else
					{
						group->indexBase = 0;
					}
					group->vertexBase = vertexBase;
					DrawGroup(pass, &*group, shadowOnly);
				}
			}
			++group;
		}
		while (group != dynamicGroups[0].end());
	}
}
