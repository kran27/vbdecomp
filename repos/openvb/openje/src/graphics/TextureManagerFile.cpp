#include "TextureManager.hpp"
#include "Texture.hpp"
#include "CubeTexture.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <dxerr8.h>
#include <stdio.h>
#include <ddraw.h>

BYTE Gfx_TextureManager::IsCubeTexture(const std::string& path)
{
	BYTE result = 0;
	FILE* input = fopen(path.c_str(), "rb");
	if (input)
	{
		DWORD signature;
		DDSURFACEDESC2 header;
		// Original ignores short reads. Unwritten short-DDS fields stay unresolved.
		fread(&signature, 4, 1, input);
		if (signature == 0x20534444)
		{
			fread(&header, sizeof(header), 1, input);
			if (header.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP) result = 1;
		}
		fclose(input);
	}
	else JE::LogDebugString("Gfx_Texture_Manager::IsCubeTex() cannot open %s\n", path.c_str());
	return result;
}

HRESULT Gfx_TextureManager::LoadTextureFromFile(DWORD* output, std::string path,
	D3DFORMAT format, D3DPOOL pool, UINT levels, DWORD filter, DWORD mipFilter, DWORD usage)
{
	HRESULT result = S_OK;
	if (!AcquireByName(output, path))
	{
		Gfx_Texture* texture;
		if (IsCubeTexture(path))
		{
			Gfx_CubeTex* cube = new Gfx_CubeTex;
			texture = cube;
			result = cube->LoadFromFile(path);
		}
		else
		{
			texture = new Gfx_Texture(1);
			result = texture->LoadFromFile(path, format, pool, levels, filter, mipFilter, usage);
		}
		if (SUCCEEDED(result))
		{
			texture->name = path;
			Register(output, texture);
		}
		else
		{
			delete texture;
			JE::LogDebugString("Gfx_Texture_Manager::LoadTextureFromFile() failed (%x %s)\n", result,
				DXGetErrorDescription8A(result));
		}
	}
	return result;
}
