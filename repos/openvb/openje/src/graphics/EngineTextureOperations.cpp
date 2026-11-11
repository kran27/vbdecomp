#include "Engine.hpp"
#include "TextureManager.hpp"
#include "Texture.hpp"

HRESULT Gfx_Engine::CreateAndAddTexture(DWORD* output, UINT width, UINT height,
	UINT mipLevels, D3DFORMAT format, D3DPOOL pool)
{
	return g_TextureManager->CreateAndAddTexture(output, width, height,
		mipLevels, format, pool);
}

HRESULT Gfx_Engine::AddTexture(DWORD* output, Gfx_Texture* texture)
{
	return g_TextureManager->AddTexture(output, texture, D3DPOOL_MANAGED);
}

BYTE Gfx_Engine::CopyTextureRect(DWORD destinationHandle, UINT destinationMip,
	const RECT* destinationRect, DWORD sourceHandle, UINT sourceMip,
	const RECT* sourceRect)
{
	return g_TextureManager->CopyTextureRect(destinationHandle, destinationMip,
		destinationRect, sourceHandle, sourceMip, sourceRect);
}

BYTE Gfx_Engine::AcquireTextureByName(DWORD* output, std::string name)
{
	return g_TextureManager->AcquireByName(output, name);
}

HRESULT Gfx_Engine::LoadTextureFromFile(DWORD* output, std::string name,
	D3DFORMAT format, D3DPOOL pool, UINT mipLevels, DWORD filter,
	DWORD mipFilter, DWORD usage)
{
	return g_TextureManager->LoadTextureFromFile(output, name, format, pool,
		mipLevels, filter, mipFilter, usage);
}

HRESULT Gfx_Engine::LoadTextureFromBuffer(DWORD* output, std::string name,
	const BYTE* data, UINT size, D3DFORMAT format, D3DPOOL pool, UINT mipLevels,
	DWORD filter, DWORD mipFilter, DWORD usage)
{
	return g_TextureManager->LoadTextureFromBuffer(output, name, data, size,
		format, pool, mipLevels, filter, mipFilter, usage);
}
