#include "D3DState.hpp"
#include "../diagnostics/DebugOutput.hpp"

HRESULT Gfx_D3D::SetTexture(DWORD stage, IDirect3DBaseTexture8* texture)
{
	DWORD count = textureStageCount;
	if (stage >= count)
	{
		JE::LogDebugString("DeviceSetTexture failed - Stage: %d < TextureStageCount: %d\n", stage, count);
		return D3DERR_NOTAVAILABLE;
	}
	HRESULT result = D3D_OK;
	if (textureStates[stage].texture != texture)
	{
		result = device->SetTexture(stage, texture);
		textureStates[stage].texture = texture;
		++statistics[8];
	}
	return result;
}
