#include "Fragment.hpp"
#include "ResourceIO.hpp"
#include "resources/ResourceSystem.hpp"
#include "graphics/SystemInterface.hpp"
#include <cstring>

namespace GUI
{
	BYTE Fragment::ReadResource(JE::Resources::ResourceFileHandle* file, int version)
	{
		if (!JE::Resources::g_pResourceSystem->Read(&tileMode, 4, file)
			|| !JE::Resources::g_pResourceSystem->Read(&frameWidth, 4, file)
			|| !JE::Resources::g_pResourceSystem->Read(&frameHeight, 4, file)
			|| !JE::Resources::g_pResourceSystem->Read(&frameRate, 4, file)
			|| !JE::Resources::g_pResourceSystem->Read(&currentFrame, 4, file)
			|| !JE::Resources::g_pResourceSystem->Read(&frameCount, 4, file)) return 0;
		std::string textureName;
		if (!ReadResourceString(textureName, file)) return 0;
		DWORD handle = 0;
		if (!g_GfxSystem->AcquireTextureByName(&handle, textureName))
		{
			JE::Resources::ResourceData* resource = JE::Resources::g_pResourceSystem->Demand(textureName.c_str(), 1);
			if (resource)
			{
				const DWORD size = resource->size;
				const BYTE* data = static_cast<const BYTE*>(resource->data);
				if (FAILED(g_GfxSystem->LoadTextureFromBuffer(&handle, textureName, data, size,
					D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 1, 1, 5, 0))) handle = 0;
				JE::Resources::g_pResourceSystem->ReleaseResource(resource);
			}
		}
		material->textureHandles[0] = handle; // No SetTextureHandle/early dirty store.
		if (!JE::Resources::g_pResourceSystem->Read(&vertices[0].z, 4, file)) return 0;
		for (int i = 1; i < 4; ++i)
		{
			std::memcpy(&vertices[i].z, &vertices[0].z, 4); // Preserve raw floating bits.
		}
		if (!JE::Resources::g_pResourceSystem->Read(&color, 4, file)) return 0;
		mesh.materialSet.SetColors(1, 1, color, 0);
		uvDirty = 1;
		mesh.materialSet.RebuildGroups();
		return Base::ReadResource(file, version);
	}
}
