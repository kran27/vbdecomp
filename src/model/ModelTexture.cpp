#include "ModelInstance.hpp"
#include "ModelCache.hpp"
#include "resources/ResourceSystem.hpp"
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
    const char* excludedMaterialNames[] = // 6FF650.
        { "", "Head", "Chest", "UpperArms", "Hands", "Legs", "Feet", "Waist", "ForeArms", "Shins" };

    BYTE ExcludedMaterial(const std::string& name) // 564AA0.
    {
        for (LONG i = 0; i < 10; ++i)
        {
            std::string excluded(excludedMaterialNames[i]);
            if (!_mbsicmp(reinterpret_cast<const unsigned char*>(name.c_str()),
                          reinterpret_cast<const unsigned char*>(excluded.c_str())))
            {
                return 1;
            }
        }
        return 0;
    }

} // namespace

namespace JE
{
    void ModelInstance::SaveSocketTextureName(DWORD slot, std::string texture)
    {
        textureNames[slot] = texture;
    }

    void ModelInstance::SetSocketModel(DWORD slot, const char* texture)
    {
        if (!instances[slot])
        {
            return;
        }
        std::string sourceName;
        if (texture && !IsStringBlank(texture, " \n\t\r"))
        {
            sourceName = texture;
        }
        else if (!slot)
        {
            sourceName = instances[0]->GetMaterial(0)->textureNames[0];
        }
        else
        {
            const LONG count = LONG(instances[slot]->data->MaterialCount());
            for (LONG i = 0; i < count; ++i)
            {
                Gfx_Material* material = instances[slot]->GetMaterial(i);
                if (material)
                {
                    std::string materialName(material->name);
                    if (!ExcludedMaterial(materialName))
                    {
                        sourceName = material->textureNames[0];
                        break;
                    }
                }
            }
        }
        if (!textureHandle && !sourceName.empty())
        {
            UINT size;
            if (strstr(sourceName.c_str(), "_LG."))
            {
                size = 512;
            }
            else if (strstr(sourceName.c_str(), "_MD."))
            {
                size = 256;
            }
            else if (strstr(sourceName.c_str(), "_SM."))
            {
                size = 128;
            }
            else
            {
                OutputDebugStringA("ModelInstance::SetSocketModel() - No size suffix on texture.");
                return;
            }
            if (FAILED(
                    g_GfxSystem->CreateAndAddTexture(&textureHandle, size, size, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED)))
            {
                OutputDebugStringA("ModelInstance::SetSocketModel() - Unable to create master stitched texture.");
                return;
            }
        }
        DWORD sourceHandle;
        Resources::ResourceMemoryStream* stream = Resources::g_pResourceSystem->OpenMemoryStream(sourceName.c_str(), 0);
        if (!stream)
        {
            return;
        }
        const HRESULT result = g_GfxSystem->LoadTextureFromBuffer(
            &sourceHandle, sourceName, static_cast<const BYTE*>(stream->resource->data), stream->resource->size,
            D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, UINT(-1), D3DX_FILTER_NONE, D3DX_FILTER_TRIANGLE, 0);
        Resources::g_pResourceSystem->CloseMemoryStream(stream);
        if (FAILED(result))
        {
            OutputDebugStringA("ModelInstance::SetSocketModel() - Unable to create master stitched texture.");
            return;
        }
        SaveSocketTextureName(slot, sourceName);
        Gfx_Texture* destination = g_GfxSystem->LookupTexture(textureHandle);
        Gfx_Texture* source = g_GfxSystem->LookupTexture(sourceHandle);
        RECT sourceRect = { 0, 0, source->frameCount > 0 ? LONG(source->frames[0].width) : 0,
                            source->frameCount > 0 ? LONG(source->frames[0].height) : 0 };
        double minimum[2] = { 0, 0 }, maximum[2] = { 0, 0 };
        Gfx_GetTextureRegion(g_pModelCache->GetTextureSlot(slot), minimum, maximum);
        RECT destinationRect;
        destinationRect.left = LONG((destination->frameCount > 0 ? destination->frames[0].width : 0) * minimum[0]);
        destinationRect.right = LONG((destination->frameCount > 0 ? destination->frames[0].width : 0) * maximum[0]);
        destinationRect.top = LONG((destination->frameCount > 0 ? destination->frames[0].height : 0) * minimum[1]);
        destinationRect.bottom = LONG((destination->frameCount > 0 ? destination->frames[0].height : 0) * maximum[1]);
        destination->CopyRect(0, &destinationRect, source, 0, &sourceRect, 0, 0);
        D3DXFilterTexture(destination->frames[0].texture, NULL, 0, D3DX_DEFAULT);
        g_GfxSystem->DeleteTexture(sourceHandle);
        for (DWORD i = 0; i < instances[slot]->data->MaterialCount(); ++i)
        {
            Gfx_Material* material = instances[slot]->GetMaterial(i);
            if (material)
            {
                material->textureHandles[0] = textureHandle;
            }
        }
    }
} // namespace JE
