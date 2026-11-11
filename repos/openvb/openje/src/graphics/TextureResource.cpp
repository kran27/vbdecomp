#include "TextureResource.hpp"
#include "SystemInterface.hpp"
#include "resources/ResourceSystem.hpp"

Gfx_TextureResourceLoader g_TextureResourceLoader = Gfx_DefaultTextureResourceLoader;

DWORD __cdecl Gfx_RequestTextureResource(const char* name, UINT mipLevels,
	DWORD filter, DWORD mipFilter, DWORD usage) //4B8180.
{
	if (g_TextureResourceLoader)
		return g_TextureResourceLoader(name, mipLevels, filter, mipFilter, usage);
	return 0;
}

DWORD __cdecl Gfx_DefaultTextureResourceLoader(const char* name, UINT mipLevels,
	DWORD filter, DWORD mipFilter, DWORD usage) //4B8580.
{
	DWORD handle = 0;
	JE::Resources::ResourceMemoryStream* stream = JE::Resources::g_pResourceSystem->OpenMemoryStream(name, 0);
	if (stream)
	{
		g_GfxSystem->LoadTextureFromBuffer(&handle, name, stream->begin,
			UINT(stream->end - stream->begin), D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
			mipLevels, filter, mipFilter, usage);
		JE::Resources::g_pResourceSystem->CloseMemoryStream(stream);
	}
	return handle;
}
