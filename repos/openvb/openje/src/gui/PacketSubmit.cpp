#include "PacketManager.hpp"
#include "graphics/MeshData.hpp"
#include "graphics/SystemInterface.hpp"

namespace GUI
{
	void CPktManager::Submit(Gfx_MeshInstance* instance, Gfx_Viewport* target, DWORD kind)
	{
		if (target)
		{
			g_GfxSystem->QueueMeshInstances(instance, target, 1);
			return;
		}
		switch (kind)
		{
		case 0:
			{
				DWORD handle = instance->mesh->materialSet.materials[0].textureHandles[0];
				std::list<Gfx_MeshInstance*>::iterator position = packets0.begin();
				std::list<Gfx_MeshInstance*>::iterator end = packets0.end();
				for (; position != end; ++position)
				{
					Gfx_MeshInstance* candidate = *position;
					if (candidate && candidate->mesh->materialSet.materials[0].textureHandles[0] == handle)
					{
						packets0.insert(position, instance);
						return;
					}
				}
				packets0.push_back(instance);
				return;
			}
		case 1:
			packets1.push_back(instance);
			return;
		case 2:
			packets2.push_back(instance);
			return;
		case 3:
			{
				DWORD color = 0, candidateColor = 0;
				Gfx_Material* material = &instance->mesh->materialSet.materials[0];
				DWORD handle = material->textureHandles[0];
				BYTE useRGB, useAlpha;
				material->GetColor(useRGB, useAlpha, color);
				std::list<Gfx_MeshInstance*>::iterator position = packets3.begin();
				for (; position != packets3.end(); ++position)
				{
					Gfx_MeshInstance* candidate = *position;
					if (candidate)
					{
						Gfx_Material* other = &candidate->mesh->materialSet.materials[0];
						DWORD candidateHandle = other->textureHandles[0];
						other->GetColor(useRGB, useAlpha, candidateColor);
						if (candidateHandle == handle && candidateColor == color)
						{
							packets3.insert(position, instance);
							return;
						}
					}
				}
				packets3.push_back(instance);
				return;
			}
		}
	}
}
