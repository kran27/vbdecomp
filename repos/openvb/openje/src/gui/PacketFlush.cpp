#include "PacketManager.hpp"
#include "Manager.hpp"
#include "graphics/MeshData.hpp"
#include "graphics/SystemInterface.hpp"

namespace GUI
{
	void CPktManager::Flush()
	{
		if (!packets1.empty())
		{
			for (std::list<Gfx_MeshInstance*>::iterator position = packets1.begin(); position != packets1.end(); ++position)
			{
				Gfx_MeshInstance* instance = *position;
				if (instance)
				{
					// The original passes its opaque GUI view owner unchanged to the
					// viewport API. Its concrete lifetime remains a separate boundary.
					Gfx_Viewport* target = reinterpret_cast<Gfx_Viewport*>(g_Manager.view);
					g_GfxSystem->QueueMeshInstances(instance, target, 1);
				}
			}
		}

		if (!packets0.empty())
		{
			DWORD count = 0;
			DWORD handle = 0;
			std::list<Gfx_MeshInstance*>::iterator position = packets0.begin();
			std::list<Gfx_MeshInstance*>::iterator first = packets0.begin();
			for (; position != packets0.end(); ++position)
			{
				Gfx_MeshInstance* instance = *position;
				if (instance)
				{
					if (!count)
						handle = instance->mesh->materialSet.materials[0].textureHandles[0];
					else if (handle == instance->mesh->materialSet.materials[0].textureHandles[0])
					{
						++count;
						continue;
					}
					else
					{
						BatchQuads(first, static_cast<LONG>(count), handle);
						handle = instance->mesh->materialSet.materials[0].textureHandles[0];
					}
					count = 1;
				}
			}
			BatchQuads(first, static_cast<LONG>(count), handle);
		}

		if (!packets3.empty())
		{
			DWORD count = 0;
			DWORD vertices = 0;
			DWORD handle = 0;
			DWORD color = 0;
			DWORD candidateColor = 0;
			BYTE useRGB, useAlpha;
			std::list<Gfx_MeshInstance*>::iterator position = packets3.begin();
			std::list<Gfx_MeshInstance*>::iterator first = packets3.begin();
			for (; position != packets3.end(); ++position)
			{
				Gfx_MeshInstance* instance = *position;
				if (instance)
				{
					Gfx_Material* material = &instance->mesh->materialSet.materials[0];
					if (!count)
					{
						handle = material->textureHandles[0];
						material->GetColor(useRGB, useAlpha, color);
						vertices = instance->mesh->vertexCount;
					}
					else
					{
						DWORD candidateHandle = material->textureHandles[0];
						material->GetColor(useRGB, useAlpha, candidateColor);
						if (handle == candidateHandle && color == candidateColor)
						{
							++count;
							vertices += instance->mesh->vertexCount;
							continue;
						}
						BatchTriangles(first, static_cast<LONG>(count), static_cast<LONG>(vertices) / 3, handle, color);
						material = &instance->mesh->materialSet.materials[0];
						handle = material->textureHandles[0];
						material->GetColor(useRGB, useAlpha, color);
						vertices = instance->mesh->vertexCount;
					}
					count = 1;
				}
			}
			BatchTriangles(first, static_cast<LONG>(count), static_cast<LONG>(vertices) / 3, handle, color);
		}

		if (!packets2.empty())
		{
			for (std::list<Gfx_MeshInstance*>::iterator position = packets2.begin(); position != packets2.end(); ++position)
			{
				Gfx_MeshInstance* instance = *position;
				if (instance)
				{
					Gfx_Viewport* target = reinterpret_cast<Gfx_Viewport*>(g_Manager.view);
					g_GfxSystem->QueueMeshInstances(instance, target, 1);
				}
			}
		}
	}
}
