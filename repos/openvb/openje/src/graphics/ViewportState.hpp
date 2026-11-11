#pragma once

#include <d3d8.h>
#include <list>

class Gfx_Viewport;
class Octree2Gfx;

namespace JE
{
	// Backend source factors with explicit borrowed viewport owners.
	// These do not construct an Engine or replace its remaining startup.
	// Fog setters and renderer-owner updates have their native bodies in Gfx_Engine.
	void SetViewportFogEnabled(Gfx_Viewport* defaultViewport, BYTE enabled,
		Gfx_Viewport* target);
	void SetViewportFogColor(Gfx_Viewport* defaultViewport, const D3DCOLOR* color,
		Gfx_Viewport* target);
	void SetViewportFogRange(Gfx_Viewport* defaultViewport, float start, float end,
		Gfx_Viewport* target);
	void SetViewportFogDensity(Gfx_Viewport* defaultViewport, float density,
		Gfx_Viewport* target);
	void DestroyViewportHeightFog(Gfx_Viewport* defaultViewport, Gfx_Viewport* target); //5BE060.
	HRESULT CreateViewportHeightFog(Gfx_Viewport* defaultViewport, D3DCOLOR color,
		float baseHeight, float baseRate, float baseAmplitude, float totalHeight,
		float totalRate, float totalAmplitude, float positionY, float maximumDensity,
		Gfx_Viewport* target); //5BE010; target override, otherwise default.
	BYTE EnableViewportHeightFog(Gfx_Viewport* defaultViewport, BYTE enabled,
		Gfx_Viewport* target); //5BE080.
	void SetViewportRendererOwner(Gfx_Viewport* defaultViewport, Octree2Gfx* owner,
		Gfx_Viewport* target);
	void SetViewportRendererColor(Gfx_Viewport* defaultViewport,
		const std::list<Gfx_Viewport*>& additionalViewports, D3DCOLOR color,
		Gfx_Viewport* target); //5BFC50; NULL changes default and every listed viewport.
	void SetGraphicsAmbientColor(const D3DCOLOR* color); //5C0200, actual cached D3D call.
	void GetGraphicsAmbientColor(D3DCOLOR* color); //5C0260, actual cached D3DRS_AMBIENT getter.
	void SetViewportFogOfWar(Gfx_Viewport* defaultViewport, BYTE& engineFlag,
		BYTE enabled, Gfx_Viewport* target); //Raw byte to renderer and engine.
	void GetGraphicsScreenDimensions(HWND window, LONG& width, LONG& height); //581590.
}
