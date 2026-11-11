#include "Texture.hpp"
#include "TextureAnimationLines.hpp"
#include <list>
#include <stdio.h>
#include <string.h>

HRESULT Gfx_Texture::LoadAnimationBuffer(const BYTE* data, int size, D3DFORMAT format,
	D3DPOOL pool, UINT levels, DWORD filter, DWORD mipFilter, DWORD usage)
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (static_cast<UINT>(size) >= 8 && !memcmp(data, "ANMTV1.0", 8))
	{
		const char* begin = reinterpret_cast<const char*>(data);
		const char* cursor = begin;
		std::list<std::string> paths;
		int line = 0;
		char text[1024];
		if (size > 0)
		{
			while (Gfx_TextureAnimationDetail::ReadLine(&cursor, size - static_cast<int>(cursor - begin),
				cursor, sizeof(text), text))
			{
				if (line && !Gfx_IsTextureAnimationWhitespace(text, " \n\t\r")) paths.push_back(text);
				++line;
				if (cursor - begin >= size) break;
			}
		}
		if (!paths.empty())
		{
			ResizeFrames(static_cast<int>(paths.size()));
			int frame = 0;
			for (std::list<std::string>::iterator path = paths.begin(); path != paths.end(); ++path, ++frame)
			{
				if (LoadFrameFromFile(*path, format, pool, levels, filter, mipFilter, usage, frame) == S_OK)
					result = S_OK;
			}
		}
	}
	return result;
}

HRESULT Gfx_Texture::LoadAnimationFile(std::string path, D3DFORMAT format, D3DPOOL pool,
	UINT levels, DWORD filter, DWORD mipFilter, DWORD usage)
{
	FILE* input = fopen(path.c_str(), "rb");
	if (!input) return D3DERR_INVALIDCALL;
	fseek(input, 0, SEEK_END);
	long size = ftell(input);
	if (size <= 0)
	{
		fclose(input);
		return D3DERR_INVALIDCALL;
	}
	BYTE* data = new BYTE[size + 1];
	if (!data)
	{
		fclose(input);
		return D3DERR_INVALIDCALL;
	}
	fseek(input, 0, SEEK_SET);
	fread(data, size, 1, input);
	data[size] = 0;
	HRESULT result = LoadAnimationBuffer(data, size + 1, format, pool, levels, filter, mipFilter, usage);
	delete[] data;
	fclose(input);
	return result;
}
