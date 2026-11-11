#include "ViewportState.hpp"
#include "Viewport.hpp"
#include "RenderManager.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include <cstring>

namespace JE
{
	void SetViewportFogEnabled(Gfx_Viewport* defaultViewport, BYTE enabled, Gfx_Viewport* target)
	{
		(target ? target : defaultViewport)->fogEnabled = enabled;
	}
	void SetViewportFogColor(Gfx_Viewport* defaultViewport, const D3DCOLOR* color, Gfx_Viewport* target)
	{
		(target ? target : defaultViewport)->fogColor = *color;
	}
	void SetViewportFogRange(Gfx_Viewport* defaultViewport, float start, float end, Gfx_Viewport* target)
	{
		Gfx_Viewport* viewport = target ? target : defaultViewport;
		std::memcpy(&viewport->fogStart, &start, sizeof(start));
		std::memcpy(&viewport->fogEnd, &end, sizeof(end));
	}
	void SetViewportFogDensity(Gfx_Viewport* defaultViewport, float density, Gfx_Viewport* target)
	{
		std::memcpy(&(target ? target : defaultViewport)->fogDensity, &density, sizeof(density));
	}
	void DestroyViewportHeightFog(Gfx_Viewport* defaultViewport, Gfx_Viewport* target)
	{
		(target ? target : defaultViewport)->renderer->ReleaseHeightFog();
	}
	HRESULT CreateViewportHeightFog(Gfx_Viewport* defaultViewport, D3DCOLOR color,
		float baseHeight, float baseRate, float baseAmplitude, float totalHeight,
		float totalRate, float totalAmplitude, float positionY, float maximumDensity,
		Gfx_Viewport* target)
	{
		return (target ? target : defaultViewport)->renderer->CreateHeightFog(color,
			baseHeight, baseRate, baseAmplitude, totalHeight, totalRate, totalAmplitude,
			positionY, maximumDensity);
	}
	BYTE EnableViewportHeightFog(Gfx_Viewport* defaultViewport, BYTE enabled, Gfx_Viewport* target)
	{
		return (target ? target : defaultViewport)->renderer->EnableHeightFog(enabled);
	}
	void SetViewportRendererOwner(Gfx_Viewport* defaultViewport, Octree2Gfx* owner, Gfx_Viewport* target)
	{
		(target ? target : defaultViewport)->renderer->floor = owner;
	}
	void SetViewportRendererColor(Gfx_Viewport* defaultViewport,
		const std::list<Gfx_Viewport*>& additionalViewports, D3DCOLOR color, Gfx_Viewport* target)
	{
		if (target)
		{
			target->renderer->shadowColor = color;
			return;
		}
		defaultViewport->renderer->shadowColor = color;
		for (std::list<Gfx_Viewport*>::const_iterator i = additionalViewports.begin();
			i != additionalViewports.end(); ++i)
		{
			(*i)->renderer->shadowColor = color;
		}
	}
	void SetGraphicsAmbientColor(const D3DCOLOR* color)
	{
		g_D3D.SetRenderState(D3DRS_AMBIENT, *color);
	}
	void GetGraphicsAmbientColor(D3DCOLOR* color)
	{
		g_D3D.GetRenderState(D3DRS_AMBIENT, color);
	}
	void SetViewportFogOfWar(Gfx_Viewport* defaultViewport, BYTE& engineFlag,
		BYTE enabled, Gfx_Viewport* target)
	{
		(target ? target : defaultViewport)->renderer->flag48 = enabled;
		engineFlag = enabled;
	}
	void GetGraphicsScreenDimensions(HWND window, LONG& width, LONG& height)
	{
		if (g_GfxSystem->Windowed())
		{
			RECT rectangle;
			GetClientRect(window, &rectangle);
			width = rectangle.right - rectangle.left;
			height = rectangle.bottom - rectangle.top;
		}
		else
		{
			Gfx_D3DMode* mode = g_GfxSystem->CurrentMode();
			width = mode->display.Width;
			height = mode->display.Height;
		}
	}
}
