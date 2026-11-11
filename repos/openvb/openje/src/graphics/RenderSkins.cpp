#include "RenderManager.hpp"
#include "SkinRender.hpp"
#include "Viewport.hpp"
#include "D3DState.hpp"

void Gfx_RenderManager::RenderSkins(UINT pass)
{
	if (pass == 0)
	{
		if (!g_D3D.currentDevice->vertexShader)
		{
			g_D3D.SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, TRUE);
		}
		g_D3D.SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
		g_D3D.SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
		g_D3D.SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		g_D3D.SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
		g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		g_D3D.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		g_D3D.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		g_D3D.SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		std::map<DWORD, std::list<Gfx_RenderSubmission16> >::iterator entry = skinSubmissions.begin();
		while (entry != skinSubmissions.end())
		{
			Gfx_SkinMesh* mesh = g_SkinMeshManager->Lookup(entry->first);
			mesh->ProcessShadowVolumeRenderSilhouette(&entry->second, projectedShadows, owner->activeCamera);
			++entry;
		}
		// The hardware flag and owner are live: callbacks above may replace them.
		if (!g_D3D.currentDevice->vertexShader)
		{
			g_D3D.SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, FALSE);
		}
		g_D3D.SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		g_D3D.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		g_D3D.SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
		g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}
	std::map<DWORD, std::list<Gfx_RenderSubmission16> >::iterator entry = skinSubmissions.begin();
	while (entry != skinSubmissions.end())
	{
		Gfx_SkinMesh* mesh = g_SkinMeshManager->Lookup(entry->first);
		std::list<Gfx_RenderSubmission16>* submissions = &entry->second;
		Gfx_HeightFog* fog = flag40 ? heightFog : 0;
		mesh->Render(pass, submissions, projectedShadows, fog, flag48);
		++entry;
	}
	g_D3D.SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
}
