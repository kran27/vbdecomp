#include "TextureManager.hpp"
#include "Texture.hpp"
#include "../JE.hpp"

HRESULT Gfx_TextureManager::LoadTextureFromBuffer(DWORD* output, std::string name,
	const BYTE* data, UINT size, D3DFORMAT format, D3DPOOL pool, UINT levels,
	DWORD filter, DWORD mipFilter, DWORD usage)
{
	HRESULT result = S_OK;
	if (!AcquireByName(output, name))
	{
		Gfx_Texture* texture = new Gfx_Texture(1);
		result = texture->LoadFromBuffer(data, size, format, pool, levels, filter, mipFilter, usage);
		if (SUCCEEDED(result))
		{
			texture->name = name;
			Register(output, texture);
		}
		else
		{
			delete texture;
			JE::LogDebugString("Gfx_Texture_Manager::LoadTextureFromBuffer() failed to load: %s\n", name.c_str());
		}
	}
	return result;
}
