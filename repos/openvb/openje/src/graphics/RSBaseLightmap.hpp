#pragma once

#include "RenderStyle.hpp"
#include "Material.hpp"
#include <d3dx8effect.h>

// Original shared70BBD8..70BBFB. techniqueCount has readers but no observed
// writes: do not invent effect GetDesc or initialize it from SDK capabilities.
struct Gfx_RS_B_LMState
{
	DWORD users;
	ID3DXEffect* normal;
	ID3DXEffect* factored;
	DWORD unknown0C, techniqueCount, technique;
	BYTE usesShader, techniqueChanged;
	BYTE unknown1A[2];
	DWORD passes, currentPass;
};
extern Gfx_RS_B_LMState g_RSBaseLightmapState;

// Actual Gfx_RS_B_LM identity;28-byte matching owner, normal SDK interfaces.
class Gfx_RS_B_LM : public Gfx_RenderStyle
{
public:
	Gfx_RS_B_LM(DWORD color, DWORD alpha, DWORD lightmapColor, DWORD lightmapAlpha,
		DWORD alphaTest); // 0x53BEF0.
	virtual ~Gfx_RS_B_LM(); // 0x536C20; ordinary shared release then base.
	virtual HRESULT Initialize(); // 0x53BF30; increments users on every call.
	virtual void Release(); // 0x535F40; unused EAX source hypothesis.
	virtual HRESULT OnLostDevice(); // 0x535F90.
	virtual HRESULT OnResetDevice(); // 0x535FC0.
	virtual DWORD TechniqueCount(); // 0x536CB0.
	virtual DWORD SelectTechnique(DWORD index); // 0x536D00.
	virtual DWORD NextTechnique(); // 0x536D30; actual virtual selection call.
	virtual BYTE UsesVertexShader(); // 0x536CC0.
	virtual DWORD PassCount(); // 0x536CD0.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD texture); // 0x53C290.
	virtual HRESULT Begin(UINT* passes); // 0x537BA0; selected instance effect.
	virtual HRESULT End(); // 0x537BC0.
	virtual HRESULT Pass(UINT pass); // 0x536CE0.

	ID3DXEffect* effect; // +04; borrowed selected normal/factored effect, initiallyNULL.
	DWORD colorOperation, alphaOperation; // +08/+0C.
	DWORD lightmapColorOperation, lightmapAlphaOperation, alphaTest; // +10/+14/+18.
};
