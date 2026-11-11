#include "RSBaseLightmap.hpp"

HRESULT Gfx_RS_B_LM::OnLostDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseLightmapState.normal)
		result = g_RSBaseLightmapState.normal->OnLostDevice();
	if (g_RSBaseLightmapState.factored)
		result = g_RSBaseLightmapState.factored->OnLostDevice();
	return result;
}

HRESULT Gfx_RS_B_LM::OnResetDevice()
{
	HRESULT result = D3DERR_INVALIDCALL;
	if (g_RSBaseLightmapState.normal)
		result = g_RSBaseLightmapState.normal->OnResetDevice();
	if (g_RSBaseLightmapState.factored)
		result = g_RSBaseLightmapState.factored->OnResetDevice();
	return result;
}

DWORD Gfx_RS_B_LM::TechniqueCount() { return g_RSBaseLightmapState.techniqueCount; }
BYTE Gfx_RS_B_LM::UsesVertexShader() { return g_RSBaseLightmapState.usesShader; }
DWORD Gfx_RS_B_LM::PassCount() { return g_RSBaseLightmapState.passes; }

DWORD Gfx_RS_B_LM::SelectTechnique(DWORD index)
{
	if (index < g_RSBaseLightmapState.techniqueCount &&
		index != g_RSBaseLightmapState.technique)
	{
		g_RSBaseLightmapState.techniqueChanged = 1;
		g_RSBaseLightmapState.technique = index;
	}
	return index;
}

DWORD Gfx_RS_B_LM::NextTechnique()
{
	DWORD next = g_RSBaseLightmapState.technique + 1;
	if (next >= g_RSBaseLightmapState.techniqueCount) next = 0;
	return SelectTechnique(next);
}

HRESULT Gfx_RS_B_LM::Begin(UINT* passes) { return effect->Begin(passes, 0); }
HRESULT Gfx_RS_B_LM::End() { return effect->End(); }

HRESULT Gfx_RS_B_LM::Pass(UINT pass)
{
	g_RSBaseLightmapState.currentPass = pass;
	return effect->Pass(pass);
}
