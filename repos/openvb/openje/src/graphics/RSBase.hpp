#pragma once

#include "RenderStyle.hpp"
#include "Material.hpp"
#include <d3dx8effect.h>

// Shared original state70BBB8..70BBD7. Production static initialization and
// publication remain unresolved: only the declaration exists in game sources.
struct Gfx_RS_BaseState
{
	DWORD users;
	ID3DXEffect* effect;
	D3DXEFFECT_DESC description;
	LPCSTR technique;
	BYTE usesShader, techniqueChanged;
	BYTE unknown16[2];
	DWORD passes, currentPass;
};
extern Gfx_RS_BaseState g_RSBaseState;

class Gfx_RS_Base : public Gfx_RenderStyle
{
public:
	Gfx_RS_Base(DWORD colorOperation, DWORD alphaOperation, DWORD alphaTest); // 0x53B9F0.
	virtual ~Gfx_RS_Base(); // 0x536AC0, including shared base cleanup and compiler EH.
	virtual HRESULT Initialize(); // 0x53BA20; increments shared users on every attempt.
	virtual void Release(); // 0x535ED0; source return hypothesis.
	virtual HRESULT OnLostDevice(); // 0x535F00.
	virtual HRESULT OnResetDevice(); // 0x535F20.
	virtual DWORD TechniqueCount(); // 0x536B30.
	virtual DWORD SelectTechnique(DWORD index); // 0x536BB0.
	virtual DWORD NextTechnique(); // 0x536BE0.
	virtual BYTE UsesVertexShader(); // 0x536B40.
	virtual DWORD PassCount(); // 0x536B50.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD texture); // 0x53BC00; texture input unused.
	virtual HRESULT Begin(UINT* passes); // 0x536B60.
	virtual HRESULT End(); // 0x536B80.
	virtual HRESULT Pass(UINT pass); // 0x536B90.

	DWORD colorOperation, alphaOperation, alphaTest; // +04/+08/+0C; original16-byte owner.
};
