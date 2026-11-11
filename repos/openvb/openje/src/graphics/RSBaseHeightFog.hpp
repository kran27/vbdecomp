#pragma once

#include "RenderStyle.hpp"
#include "Material.hpp"
#include <d3dx8effect.h>

struct Gfx_RS_BaseHeightFogState
{
	DWORD users;
	ID3DXEffect* effect;
	D3DXEFFECT_DESC description;
	LPCSTR technique;
	BYTE usesShader, techniqueChanged;
	BYTE padding16[2];
	DWORD passes, currentPass;
};
extern Gfx_RS_BaseHeightFogState g_RSBaseHeightFogState; //70BC1C.

class Gfx_RS_Base_HeightFog : public Gfx_RenderStyle
{
public:
	Gfx_RS_Base_HeightFog(DWORD color, DWORD alpha, DWORD test); //53CC80.
	virtual ~Gfx_RS_Base_HeightFog(); //536ED0.
	virtual HRESULT Initialize(); //53CCA0.
	virtual void Release(); //536060.
	virtual HRESULT OnLostDevice(); //536090.
	virtual HRESULT OnResetDevice(); //5360B0.
	virtual DWORD TechniqueCount(); //536F40.
	virtual DWORD SelectTechnique(DWORD index); //536FC0.
	virtual DWORD NextTechnique(); //536FF0.
	virtual BYTE UsesVertexShader(); //536F50.
	virtual DWORD PassCount(); //536F60.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD fogBits); //53CE90, RET8.
	virtual HRESULT Begin(UINT* passes); //536F70.
	virtual HRESULT End(); //536F90.
	virtual HRESULT Pass(UINT pass); //536FA0.

	DWORD colorOperation, alphaOperation, alphaTest;
};
