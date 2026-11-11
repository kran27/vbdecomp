#pragma once

#include <windows.h>
#include <d3dx8.h>

struct Gfx_D3DMode
{
	Gfx_D3DMode(); // 0x5024F0; padding+29..31 remains unwritten.
	~Gfx_D3DMode() {} // Shared empty callback; real compiler supplies array lifetime.
	D3DDISPLAYMODE display; // SDK display value, +00..0F.
	D3DFORMAT backBufferFormat, depthStencilFormat;
	DWORD behavior;
	BYTE stencil;
};

// Observed24-byte temporary packet. Only display/backBuffer are initialized
// before vector insertion; all other bytes must remain unwritten until policy
// assigns them. In particular, accepted is a raw byte, not an initialized bool.
struct Gfx_D3DFormatInfo
{
	D3DFORMAT display, backBuffer;
	DWORD behavior;
	BYTE accepted;
	BYTE unknown0D[3];
	D3DFORMAT depth;
	DWORD unknown14;
};

// SDK-derived device/adapter concepts, with the game's actual dynamic-array ABI.
// These are POD array payloads; Shutdown owns the separately allocated children.
struct Gfx_D3DDeviceInfo
{
	D3DDEVTYPE type;
	char description[4]; // Inline HAL/REF/NUL, not an owned string or pointer.
	D3DCAPS8 caps;
	BYTE windowed, guardBand, vertexShader, pixelShader;
	DWORD unknown0E0;
	DWORD modeCount; // +0E4.
	Gfx_D3DMode* modes; // +0E8; owned array with real C++ lifetime.
};

struct Gfx_D3DAdapterInfo
{
	D3DADAPTER_IDENTIFIER8 identifier;
	DWORD deviceCount; // +42C.
	Gfx_D3DDeviceInfo* devices; // +430; owned POD array.
	D3DDISPLAYMODE desktop; // +434.
	D3DFORMAT windowDepthStencil; // +444.
	DWORD windowBehavior; // +448; not defaulted on acquisition.
};

struct Gfx_D3DTextureState
{
	IDirect3DBaseTexture8* texture;
	DWORD values[29];
};

// Descriptive declaration of original global0x7082F8, through known +0x6B8.
// This is the legacy state ABI, not the historical host D3DContext layout.
// The full object's extent remains unresolved. GraphicsStorage connects the
// native pre-main Initialize/atexit Shutdown policy; full startup is separate.
class Gfx_D3D
{
public:
	Gfx_D3D* Initialize(BYTE allowReference); // 0x505170; returns this after actual enumeration.
	void EnumerateDevices(BYTE allowReference); // 0x5047A0.
	bool ChooseDepthStencil(D3DFORMAT& output, UINT adapter, D3DDEVTYPE type,
		D3DFORMAT adapterFormat, D3DFORMAT backBuffer, DWORD unused0, DWORD unused1); // 0x5026C0.
	bool FindMode(DWORD& output, UINT adapter, UINT deviceIndex, DWORD width,
		DWORD height, BYTE color32, DWORD refreshRate); // 0x502870.
	void Shutdown(); // 0x503F80; release device, D3D, modes/devices/adapters.
	void ReleaseDevice(); // 0x503C30; preserves ready, stage count and borrowed slots.
	HRESULT CreateDevice(HWND owner, UINT adapter, UINT deviceIndex, UINT modeIndex,
		BYTE windowed, BYTE multithreaded); // 0x503920; returns the matrix-stack HRESULT.
	HRESULT ResetDevice(const D3DPRESENT_PARAMETERS* parameters); // 0x503CD0; optional52-byte descriptor, success replays current states.
	HRESULT SetMode(DWORD mode); //503D90; unchecked selected-device mode, full-screen reset.
	HRESULT SetWindowMode(DWORD width, DWORD height); //503E30; desktop-derived window mode.
	HRESULT InitializeDefaultRenderStates(); // 0x502CB0; explicit0 return.
	DWORD InitializeDefaultTextureStates(); // 0x502F40; final unsigned loop value.
	HRESULT ApplyDefaultRenderStates(); // 0x503120; last SDK HRESULT.
	DWORD ApplyDefaultTextureStates(); // 0x503170; live count after the last stage, or0.
	DWORD ApplyCachedStates(BYTE defaults); // 0x503220; raw EAX result, see function record.
	HRESULT SetTexture(DWORD stage, IDirect3DBaseTexture8* texture); // 0x503390.
	HRESULT SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix); // 0x503360.
	HRESULT DrawPrimitive(D3DPRIMITIVETYPE type, UINT startVertex, UINT primitiveCount); //490060; counters publish before SDK call.
	HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE type, UINT minimumIndex, UINT vertexCount,
		UINT startIndex, UINT primitiveCount); //0x4900A0; actual draw/statistics wrapper.
	HRESULT GetRenderState(D3DRENDERSTATETYPE state, DWORD* value); // 0x503300; cached value and explicit success0.
	HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value); // 0x503320; cache updates even on SDK failure.
	HRESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE state, DWORD value); // 0x503400.
	void DescribeTextureFormat(int format, DWORD* bits, char* name); // 0x503450; signed selector.
	BYTE* DescribeDepthStencilFormat(D3DFORMAT format, DWORD* depth, DWORD* stencil, BYTE* lockable); // 0x5037B0; thiscall/RET16.
	DWORD GetFreeTextureMemory(); // 0x502BB0; live cache reload after diagnostics.
	HRESULT ScreenCapture(const char* filename); //502950, ordinary D3D-owner member.
	void UpdateStatistics(); // 0x502BE0; uses the unrounded x87 product.

	HWND window; // +000.
	IDirect3D8* direct3D; // +004.
	IDirect3DDevice8* device; // +008.
	D3DPRESENT_PARAMETERS presentation; // +00C.
	D3DXMATRIX identity; // +040; actual SDK matrix, unchanged ABI.
	ID3DXMatrixStack* matrixStack; // +080; actual SDK interface.
	DWORD adapterCount; // +084.
	Gfx_D3DAdapterInfo* adapters; // +088; original array owner, not a vector.
	DWORD currentAdapter, currentDeviceIndex; // +08C/+090.
	Gfx_D3DDeviceInfo* currentDevice; // +094; borrowed.
	DWORD currentModeIndex; // +098.
	Gfx_D3DMode* currentMode; // +09C; borrowed.
	BYTE requestedWindowed; // +0A0.
	BYTE unknown0A1[3];
	DWORD value0A4, vertexLimit, value0AC; // +0A4/+0A8/+0AC.
	union
	{
		DWORD statistics[16]; // +0B0; raw counter projection.
		float framesPerSecond; // +0B0; actual floating-point statistics output.
	};
	BYTE ready; // +0F0.
	BYTE unknown0F1[3];
	DWORD defaultRenderStates[174]; // +0F4.
	DWORD renderStates[174]; // +3AC.
	DWORD textureStageCount; // +664.
	Gfx_D3DTextureState* defaultTextureStates; // +668; owned.
	Gfx_D3DTextureState* textureStates; // +66C; owned.
	Gfx_D3DMode windowMode; // +670.
	BYTE verticalSync; // +690.
	BYTE unknown691[7];
	LARGE_INTEGER performanceFrequency; // +698.
	BYTE unknown6A0[8];
	LARGE_INTEGER performanceCounter; // +6A8; actual QPC destination.
	float inverseFrequency; // +6B0.
	float previousStatisticsTime; // +6B4; initialized to0 by505170.
	float currentStatisticsTime; // +6B8; not initialized by505170.
	// Final4 bytes are not explained. The next referenced global is +6C0;
	// a complete source-level owner extent remains an inference, not a match.
};

//Sole canonical global. Startup/GraphicsStorage connects original6419B0/645E70
//pre-main SDK lifetime through real compiler/atexit services. Full startup stays
//separate; the scene explicitly initializes/shuts down its own entry/global.
//Do not create a second vertex-limit owner or infer SDK readiness from storage.
extern Gfx_D3D g_D3D;

// Separate absolute storage cleared by the default-state APIs. Only the write
// extents are established; no list/counter meaning or production owner is
// inferred from these reset-only xrefs. Definitions remain a startup boundary.
extern BYTE g_D3DRenderResetBytes[174]; // Original0x708158; includes a final WORD.
extern BYTE g_D3DTextureResetBytes[232]; // Original0x708208.
extern DWORD g_D3DTextureResetValue0; // Original0x7082F0.
extern DWORD g_D3DTextureResetValue1; // Original0x7082F4.
