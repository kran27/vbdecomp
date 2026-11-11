#include "TextureManager.hpp"
#include "Texture.hpp"
#include "../JE.hpp"
#include <dxerr8.h>

HRESULT Gfx_TextureManager::AddTexture(DWORD* output, Gfx_Texture* source, D3DPOOL pool)
{
	if (source && !AcquireByName(output, source->GetDiagnosticName()))
	{
		Gfx_Texture* texture = new Gfx_Texture(1);
		texture->name = source->GetDiagnosticName();
		texture->ResizeFrames(source->frameCount);
		BYTE created = 0;
		for (int frame = 0; frame < source->frameCount; ++frame)
		{
			Gfx_TextureFrame& input = source->frames[frame];
			HRESULT result = texture->CreateTexture(input.width, input.height, input.mipCount,
				input.format, pool, 0, frame);
			if (SUCCEEDED(result))
			{
				texture->SetFrameName(source->GetFrameName(frame), frame);
				if (pool) texture->CopyRect(0, NULL, source, 0, NULL, frame, frame);
				else texture->UpdateDefaultTexture(source, frame, frame);
				created = 1;
			}
			else
				JE::LogDebugString("Gfx_Texture_Manager::AddTexture - CreateTexture Failed (%x %s)\n",
					result, DXGetErrorDescription8A(result));
		}
		if (created) Register(output, texture);
		// Original leaks the new owner when no frame was created and returns S_OK.
	}
	return S_OK;
}

HRESULT Gfx_TextureManager::CreateAndAddTexture(DWORD* output, UINT width, UINT height,
	UINT levels, D3DFORMAT format, D3DPOOL pool)
{
	Gfx_Texture source(1);
	HRESULT result = source.CreateSystemTexture("", width, height, levels, format, 0);
	if (SUCCEEDED(result)) AddTexture(output, &source, pool);
	else
		JE::LogDebugString("Gfx_Texture_Manager::CreateAndAddTexture - CreateSysMemTexture Failed (%x %s)\n",
			result, DXGetErrorDescription8A(result));
	return result; // Original returns the temporary creation result, not AddTexture's.
}

BYTE Gfx_TextureManager::CopyTextureRect(DWORD destinationHandle, UINT destinationMip,
	const RECT* destinationRect, DWORD sourceHandle, UINT sourceMip,
	const RECT* sourceRect)
{
	BYTE result = 0;
	Gfx_Texture* destination = Lookup(destinationHandle);
	Gfx_Texture* source = Lookup(sourceHandle);
	if (destination && source)
		result = destination->CopyRect(destinationMip, destinationRect, source,
			sourceMip, sourceRect, 0, 0);
	return result;
}
