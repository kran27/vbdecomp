#pragma once

#include <windows.h>
#include <d3dx8.h>
#include <list>

class Gfx_Material;
class Gfx_Light;
class Gfx_TextureParameters;

// Actual fourteen-slot proprietary style interface. Its Begin/End/Pass slots
// wrap SDK effects; this object is never an ID3DXEffect layout or pointer.
class Gfx_RenderStyle
{
public:
	virtual ~Gfx_RenderStyle() = 0; // +00 pure slot; ordinary base dtor chunk0x535EC0.
	virtual HRESULT Initialize() = 0; // +04.
	virtual void Release() = 0; // +08; source return hypothesis, unused EAX.
	virtual HRESULT OnLostDevice() = 0; // +0C.
	virtual HRESULT OnResetDevice() = 0; // +10.
	virtual DWORD TechniqueCount() = 0; // +14.
	virtual DWORD SelectTechnique(DWORD index) = 0; // +18.
	virtual DWORD NextTechnique() = 0; // +1C.
	virtual BYTE UsesVertexShader() = 0; // +20.
	virtual DWORD PassCount() = 0; // +24.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD heightFog) = 0; //+28; Gfx_HeightFog* bits.
	virtual HRESULT Begin(UINT* passes) = 0; // +2C.
	virtual HRESULT End() = 0; // +30.
	virtual HRESULT Pass(UINT pass) = 0; // +34.

	HRESULT SetTextureTransform(DWORD stage, BYTE flags,
		const Gfx_TextureParameters* parameters, float seconds); //5434E0; shared nonvirtual member.
};

// Nonpolymorphic24-byte selector; slot identities and source flag names are
// recovered from the actual templates/callers; final argument is a borrowed fog object.
class Gfx_RenderStyleController
{
public:
	explicit Gfx_RenderStyleController(Gfx_RenderStyle* const* initial); // 0x506110.
	HRESULT SetMaterial(Gfx_Material* material, BYTE techniqueFlag, BYTE fogOfWar,
		BYTE heightFogEnabled, DWORD heightFog); //506140; final pointer transported as DWORD.

	Gfx_RenderStyle* styles[5]; // +00..10, borrowed global style objects.
	DWORD activeIndex; // +14; unchecked after callbacks, reloaded by consumers.
};

extern Gfx_RenderStyleController* g_RenderStyles[34]; // 0x6FBFC8,32 entries+2 NULL.
extern Gfx_RenderStyleController g_ShadowRenderStyle; // 0x708A8C, table entry27.

void Gfx_ResetLights(); // 0x506290; live active-light count after each SDK call.
void Gfx_ApplyLights(const std::list<Gfx_Light*>* lights, const D3DXVECTOR4* extra,
	DWORD viewport); // 0x5062D0; viewport bits passed to the actual SubmitLight API.

inline Gfx_RenderStyle::~Gfx_RenderStyle() {}
