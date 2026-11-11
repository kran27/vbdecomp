#include "RSSKBaseLightmap.hpp"

HRESULT Gfx_RS_SK_B_LM::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSSKBaseLightmapState.normal)
		result = g_RSSKBaseLightmapState.normal->OnLostDevice();
	if (g_RSSKBaseLightmapState.factored)
		result = g_RSSKBaseLightmapState.factored->OnLostDevice();
	return result;
}

HRESULT Gfx_RS_SK_B_LM::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSSKBaseLightmapState.normal)
		result = g_RSSKBaseLightmapState.normal->OnResetDevice();
	if (g_RSSKBaseLightmapState.factored)
		result = g_RSSKBaseLightmapState.factored->OnResetDevice();
	return result;
}

DWORD Gfx_RS_SK_B_LM::TechniqueCount() { return g_RSSKBaseLightmapState.techniqueCount; }
BYTE Gfx_RS_SK_B_LM::UsesVertexShader() { return g_RSSKBaseLightmapState.usesShader; }
DWORD Gfx_RS_SK_B_LM::PassCount() { return g_RSSKBaseLightmapState.passes; }

DWORD Gfx_RS_SK_B_LM::SelectTechnique(DWORD index)
{
	if (index < g_RSSKBaseLightmapState.techniqueCount &&
		index != g_RSSKBaseLightmapState.technique)
	{
		g_RSSKBaseLightmapState.techniqueChanged = 1;
		g_RSSKBaseLightmapState.technique = index;
	}
	return index;
}

DWORD Gfx_RS_SK_B_LM::NextTechnique()
{
	DWORD next = g_RSSKBaseLightmapState.technique + 1;
	if (next >= g_RSSKBaseLightmapState.techniqueCount) next = 0;
	return SelectTechnique(next);
}

HRESULT Gfx_RS_SK_B_LM::Begin(UINT* passes) { return effect->Begin(passes, 0); }
HRESULT Gfx_RS_SK_B_LM::End() { return effect->End(); }

HRESULT Gfx_RS_SK_B_LM::Pass(UINT pass)
{
	g_RSSKBaseLightmapState.currentPass = pass;
	return effect->Pass(pass);
}
