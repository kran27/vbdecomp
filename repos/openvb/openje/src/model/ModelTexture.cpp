#include "ModelInstance.hpp"
#include "ModelCache.hpp"
#include "ModelServices.hpp"
#include "graphics/ModelInstance.hpp"
#include "graphics/ModelData.hpp"
#include "graphics/Material.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"
#include "graphics/TextureAtlas.hpp"
#include "graphics/StringFormat.hpp"
#include <mbstring.h>
#include <string.h>

namespace
{
	BYTE ExcludedMaterial(const std::string& name) // 0x564AA0, original6FF650.
	{
		const char* excluded[] =
		{
			"", "Head", "Chest", "UpperArms", "Hands", "Legs", "Feet", "Waist", "ForeArms", "Shins"
		};
		for (DWORD i = 0; i < sizeof(excluded) / sizeof(excluded[0]); ++i)
			if (!_mbsicmp(reinterpret_cast<const unsigned char*>(name.c_str()),
				reinterpret_cast<const unsigned char*>(excluded[i]))) return 1;
		return 0;
	}

}

namespace JE
{
	void ModelInstance::SaveSocketTextureName(DWORD slot, std::string texture)
	{
		textureNames[slot] = texture;
	}

	void ModelInstance::SetSocketModel(DWORD slot, const char* texture)
	{
		Gfx_ModelInstance* model = instances[slot];
		if (!model) return;
		std::string sourceName;
		if (texture && !IsStringBlank(texture, " \n\t\r")) sourceName = texture;
		else if (!slot) sourceName = instances[0]->GetMaterial(0)->textureNames[0];
		else
		{
			DWORD count = model->data->MaterialCount();
			for (DWORD i = 0; i < count; ++i)
			{
				Gfx_Material* material = model->GetMaterial(i);
				if (material && !ExcludedMaterial(material->name))
				{
					sourceName = material->textureNames[0];
					break;
				}
			}
		}
		if (!textureHandle && !sourceName.empty())
		{
			UINT size;
			if (strstr(sourceName.c_str(), "_LG.")) size = 512;
			else if (strstr(sourceName.c_str(), "_MD.")) size = 256;
			else if (strstr(sourceName.c_str(), "_SM.")) size = 128;
			else
			{
				OutputDebugStringA("ModelInstance::SetSocketModel() - No size suffix on texture.");
				return;
			}
			if (FAILED(g_GfxSystem->CreateAndAddTexture(&textureHandle, size, size, 0,
				D3DFMT_A8R8G8B8, D3DPOOL_MANAGED)))
			{
				OutputDebugStringA("ModelInstance::SetSocketModel() - Unable to create master stitched texture.");
				return;
			}
		}
		DWORD sourceHandle;
		HRESULT result;
		if (!ModelServices::LoadSocketTexture(&sourceHandle, sourceName, &result)) return;
		if (FAILED(result))
		{
			OutputDebugStringA("ModelInstance::SetSocketModel() - Unable to create master stitched texture.");
			return;
		}
		SaveSocketTextureName(slot, sourceName);
		Gfx_Texture* destination = g_GfxSystem->LookupTexture(textureHandle);
		Gfx_Texture* source = g_GfxSystem->LookupTexture(sourceHandle);
		RECT sourceRect = {0, 0,
			source->frameCount > 0 ? LONG(source->frames[0].width) : 0,
			source->frameCount > 0 ? LONG(source->frames[0].height) : 0};
		double minimum[2] = {0, 0}, maximum[2] = {0, 0};
		Gfx_GetTextureRegion(g_pModelCache->Number(slot, "TextureSlot", 1), minimum, maximum);
		UINT width = destination->frameCount > 0 ? destination->frames[0].width : 0;
		UINT height = destination->frameCount > 0 ? destination->frames[0].height : 0;
		RECT destinationRect = {LONG(width * minimum[0]), LONG(height * minimum[1]),
			LONG(width * maximum[0]), LONG(height * maximum[1])};
		destination->CopyRect(0, &destinationRect, source, 0, &sourceRect, 0, 0);
		D3DXFilterTexture(destination->frames[0].texture, NULL, 0, D3DX_DEFAULT);
		g_GfxSystem->DeleteTexture(sourceHandle);
		for (DWORD i = 0; i < model->data->MaterialCount(); ++i)
		{
			Gfx_Material* material = model->GetMaterial(i);
			if (material) material->textureHandles[0] = textureHandle;
		}
	}
}
