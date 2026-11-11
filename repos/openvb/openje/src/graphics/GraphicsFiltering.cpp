#include "GraphicsFiltering.hpp"
#include "D3DState.hpp"

namespace JE
{
	void SetGraphicsLinearFiltering(BYTE enabled)
	{
		const DWORD filter = enabled ? D3DTEXF_LINEAR : D3DTEXF_POINT;
		DWORD stage = 0;
		//The original reads the live selected-device cap, not textureStageCount,
		//and reloads it after both cache/device calls on every iteration.
		while (stage < g_D3D.currentDevice->caps.MaxTextureBlendStages)
		{
			g_D3D.SetTextureStageState(stage, D3DTSS_MAGFILTER, filter);
			g_D3D.SetTextureStageState(stage, D3DTSS_MINFILTER, filter);
			++stage;
		}
	}

	void SetGraphicsMipFiltering(BYTE enabled)
	{
		const DWORD filter = enabled ? D3DTEXF_LINEAR : D3DTEXF_NONE;
		DWORD stage = 0;
		while (stage < g_D3D.currentDevice->caps.MaxTextureBlendStages)
		{
			g_D3D.SetTextureStageState(stage, D3DTSS_MIPFILTER, filter);
			++stage;
		}
	}
}
