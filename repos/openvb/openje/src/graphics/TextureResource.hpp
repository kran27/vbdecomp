#pragma once

#include <windows.h>

typedef DWORD (__cdecl *Gfx_TextureResourceLoader)(const char* name, UINT mipLevels,
	DWORD filter, DWORD mipFilter, DWORD usage);

extern Gfx_TextureResourceLoader g_TextureResourceLoader; //Original6FAFF0 ->4B8580.
DWORD __cdecl Gfx_RequestTextureResource(const char* name, UINT mipLevels,
	DWORD filter, DWORD mipFilter, DWORD usage); //4B8180, misnamed _matherr in IDA.
DWORD __cdecl Gfx_DefaultTextureResourceLoader(const char* name, UINT mipLevels,
	DWORD filter, DWORD mipFilter, DWORD usage); //4B8580, actual resource/Graphics join.
