#include "RenderManager.hpp"
#include "HeightFog.hpp"

HRESULT Gfx_RenderManager::CreateHeightFog(D3DCOLOR color, float baseHeight,
	float baseRate, float baseAmplitude, float totalHeight, float totalRate,
	float totalAmplitude, float positionY, float maximumDensity)
{
	if (!heightFog) heightFog = new Gfx_HeightFog;
	heightFog->SetPosition(0.0f, positionY, 0.0f);
	return heightFog->CreateFog(color, baseHeight, baseRate, baseAmplitude,
		totalHeight, totalRate, totalAmplitude, maximumDensity);
}
