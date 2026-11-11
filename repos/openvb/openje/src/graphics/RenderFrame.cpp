#include "RenderManager.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "RenderFloor.hpp"

void Gfx_RenderManager::Render(BYTE clear)
{
	g_D3D.SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	g_D3D.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	g_D3D.SetRenderState(D3DRS_ALPHAREF, 0x7F);
	Upload(0, 0);
	Upload(0, 1);
	RenderSkins(0);
	g_D3D.SetRenderState(D3DRS_ALPHAREF, 0);
	if (projectedShadows && g_D3D.currentMode->stencil)
	{
		RenderProjectedShadows();
	}
	else if (flag49 && floor)
	{
		floor->RenderFloorCB();
	}
	g_D3D.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	RenderSkins(1);
	Upload(1, 0);
	g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTCOLOR);
	g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	RenderSkins(2);
	Upload(2, 0);
	g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT);
	RenderSkins(3);
	Upload(3, 0);
	g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
	RenderSkins(4);
	Upload(4, 0);
	g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	RenderSkins(5);
	Upload(5, 0);
	if (g_GfxSystem->DebugLightsEnabled())
	{
		RenderDebugLights();
	}
	if (clear)
	{
		Clear();
		debugLights.clear();
		RecycleDebugSpheres();
	}
}
