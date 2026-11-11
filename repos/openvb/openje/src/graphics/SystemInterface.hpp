#pragma once

#include <windows.h>
#include <d3d8.h>
#include <string>

struct Gfx_D3DMode;
class Gfx_Viewport;
class Gfx_Camera;
class Gfx_CamCtrl;
class Gfx_ModelData526620; // Opaque nonpolymorphic owner; source name unresolved.
class Gfx_ModelInstance;
class Gfx_Texture;
class Gfx_MeshData;
class Gfx_MeshInstance;
class Gfx_Light;

// All120 operations correspond to actual original table0x6AE7EC entries.
// Names and non-unique source spellings are analysis hypotheses. H-RETURN
// explicitly marks unused void-return hypotheses, never incidental EAX credit.
// H-TYPE marks unresolved pointer/ref/enum/class or source-type spelling.
// Only evidenced calls are integrated; this does not implement Gfx_Engine or
// claim complete behavior/type validation for the other interface operations.
class Gfx_System_Interface
{
public:
	Gfx_System_Interface(); //Inlined publication before Engine5BFE10 members.
	// +0x00 ->0x5c1e40; stack arguments4 bytes.
	virtual ~Gfx_System_Interface() = 0;
	// +0x04 ->0x5c0440; stack arguments0 bytes.
	virtual DWORD AdapterCount() = 0;
	// +0x08 ->0x5c0450; stack arguments4 bytes.
	virtual DWORD DeviceCount(UINT adapter) = 0;
	// +0x0C ->0x5c0480; stack arguments8 bytes.
	virtual DWORD ModeCount(UINT adapter, UINT device) = 0;
	// +0x10 ->0x5c04d0; stack arguments8 bytes.
	virtual Gfx_D3DMode* Modes(UINT adapter, UINT device) = 0;
	// +0x14 ->0x5c0080; stack arguments0 bytes.
	virtual BYTE VerticalSync() = 0;
	// +0x18 ->0x5bf770; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void SetVerticalSync(BYTE value) = 0;
	// +0x1C ->0x5c0090; stack arguments0 bytes.
	virtual BYTE State68() = 0;
	// +0x20 ->0x5c0c30; stack arguments24 bytes.
	virtual BYTE CreateDevice(HWND window, UINT adapter, UINT device, UINT mode, BYTE windowed, BYTE flag) = 0;
	// +0x24 ->0x5c1b30; stack arguments0 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void ShutdownDevice() = 0;
	// +0x28 ->0x5c05a0; stack arguments28 bytes.
	virtual bool FindMode(DWORD& outMode, UINT adapter, UINT device, DWORD width, DWORD height,
		BYTE color32, DWORD refresh) = 0;
	// +0x2C ->0x5bf7e0; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void SetMode(DWORD mode) = 0;
	// +0x30 ->0x5bf8c0; stack arguments20 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void SetWindowMode(DWORD width, DWORD height, BYTE windowed, BYTE color32, DWORD refresh) = 0;
	// +0x34 ->0x5c0520; stack arguments0 bytes.
	virtual Gfx_D3DMode* CurrentMode() = 0;
	// +0x38 ->0x5c00a0; stack arguments0 bytes.
	virtual BYTE Windowed() = 0;
	// +0x3C ->0x5c0860; stack arguments12 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void SetColor(BYTE red, BYTE green, BYTE blue) = 0;
	// +0x40 ->0x5c00b0; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void SetViewportFlags(DWORD flags) = 0;
	// +0x44 ->0x5c00c0; stack arguments0 bytes.
	virtual DWORD ModeIndex() = 0;
	// +0x48 ->0x5c00d0; stack arguments0 bytes.
	virtual HWND Window() = 0;
	// +0x4C ->0x5c00e0; stack arguments0 bytes.
	virtual DWORD ActiveLightLimit() = 0;
	// +0x50 ->0x5c00f0; stack arguments0 bytes.
	virtual DWORD MaximumActiveLights() = 0;
	// +0x54 ->0x5c0530; stack arguments4 bytes.
	// Integer result intent supported; exact typedef remains a hypothesis.
	virtual DWORD SetActiveLightLimit(DWORD limit) = 0;
	// +0x58 ->0x5c0100; stack arguments0 bytes.
	virtual DWORD MaximumVertexBlendMatrices() = 0;
	// +0x5C ->0x5c0560; stack arguments8 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void MaximumTextureSize(DWORD& width, DWORD& height) = 0;
	// +0x60 ->0x5c0110; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void SetRendererFlag56(BYTE value) = 0;
	// +0x64 ->0x5c0130; stack arguments0 bytes.
	virtual BYTE RendererFlag56() = 0;
	// +0x68 ->0x5bdb20; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void EnableProjectedShadow(BYTE value) = 0;
	// +0x6C ->0x5c0140; stack arguments0 bytes.
	virtual BYTE ProjectedShadowEnabled() = 0;
	// +0x70 ->0x5bdb70; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void SetLinearFiltering(BYTE enabled) = 0;
	// +0x74 ->0x5bdc10; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void SetMipFiltering(BYTE enabled) = 0;
	// +0x78 ->0x5bddb0; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void AdvanceTime(DWORD deltaMilliseconds) = 0;
	// +0x7C ->0x5c0150; stack arguments0 bytes.
	virtual DWORD DeltaMilliseconds() = 0;
	// +0x80 ->0x5c0160; stack arguments0 bytes.
	virtual DWORD TimeMilliseconds() = 0;
	// +0x84 ->0x5c0170; stack arguments0 bytes.
	// Float storage/consumer proven; original float versus double declaration remains non-unique.
	virtual float TimeSeconds() = 0;
	// +0x88 ->0x5bddf0; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void SetTimeMilliseconds(DWORD value) = 0;
	// +0x8C ->0x5c1940; stack arguments0 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void RenderFrame() = 0;
	// +0x90 ->0x5c0180; stack arguments0 bytes.
	virtual Gfx_Viewport* DefaultViewport() = 0;
	// +0x94 ->0x5c0190; stack arguments0 bytes.
	virtual Gfx_Viewport* CurrentViewport() = 0;
	// +0x98 ->0x5c01a0; stack arguments0 bytes.
	virtual Gfx_Viewport* PostViewport() = 0;
	// +0x9C ->0x5c1b00; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void AddViewport(Gfx_Viewport* viewport) = 0;
	// +0xA0 ->0x5c01b0; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void SetCamera(Gfx_Camera* camera) = 0;
	// +0xA4 ->0x5c1b20; stack arguments4 bytes.
	// Void is a documented source hypothesis; incidental registers are not API returns.
	virtual void SetCameraController(Gfx_CamCtrl* control) = 0;
	// +0xA8 ->0x5c01d0; stack arguments0 bytes.
	virtual Gfx_Camera* Camera() = 0;
	// +0xAC ->0x5c01e0; stack arguments0 bytes.
	virtual Gfx_CamCtrl* CameraController() = 0;
	// +0xB0 ->0x5c08a0; stack arguments4 bytes.
	virtual DWORD RegisterModelData(Gfx_ModelData526620* object) = 0;
	// +0xB4 ->0x5c08b0; stack arguments4 bytes.
	virtual DWORD ModelDataReferences(DWORD* handle) = 0;
	// +0xB8 ->0x5c08c0; stack arguments4 bytes.
	virtual DWORD ReleaseModelData(DWORD* handle) = 0;
	// +0xBC ->0x5c08d0; stack arguments4 bytes.
	virtual Gfx_ModelInstance* CreateModelInstance(DWORD handle) = 0;
	// +0xC0 ->0x5c0a00; stack arguments56 bytes.
	virtual HRESULT LoadTextureFromFile(DWORD* outHandle, std::string name, D3DFORMAT format,
		D3DPOOL pool, UINT mipLevels, DWORD filter, DWORD mipFilter, DWORD usage) = 0;
	// +0xC4 ->0x5c0a90; stack arguments64 bytes.
	virtual HRESULT LoadTextureFromBuffer(DWORD* outHandle, std::string name, const BYTE* data,
		UINT size, D3DFORMAT format, D3DPOOL pool, UINT mipLevels, DWORD filter, DWORD mipFilter,
		DWORD usage) = 0;
	// +0xC8 ->0x5c08e0; stack arguments24 bytes.
	virtual HRESULT CreateAndAddTexture(DWORD* outHandle, UINT width, UINT height, UINT mipLevels,
		D3DFORMAT format, D3DPOOL pool) = 0;
	// +0xCC ->0x5c08f0; stack arguments8 bytes.
	virtual HRESULT AddTexture(DWORD* outHandle, Gfx_Texture* texture) = 0;
	// +0xD0 ->0x5c0910; stack arguments4 bytes.
	virtual DWORD DeleteTexture(DWORD handle) = 0;
	// +0xD4 ->0x5c0920; stack arguments24 bytes.
	virtual BYTE CopyTextureRect(DWORD destHandle, UINT destMip, const RECT* destRect, DWORD srcHandle,
		UINT srcMip, const RECT* srcRect) = 0;
	// +0xD8 ->0x5c0930; stack arguments4 bytes.
	virtual Gfx_Texture* LookupTexture(DWORD handle) = 0;
	//55,+DC,5C0970; acquired reference; BYTE/bool H-TYPE
	virtual BYTE AcquireTextureByName(DWORD* outHandle, std::string name) = 0;
	//56,+E0,5C0940; common packet-data base versus mesh H-TYPE
	virtual bool CreateGeometryData(Gfx_MeshData* data) = 0;
	//57,+E4,5C0950; reads data+4; H-RETURN/base H-TYPE
	virtual void ReleaseGeometryData(Gfx_MeshData* data) = 0;
	//58,+E8,5C05B0;40-byte stride; H-RETURN
	virtual void QueueMeshInstances(Gfx_MeshInstance* instances, Gfx_Viewport* viewport, UINT count) = 0;
	//59,+EC,5C05F0; renderable+36 owner; roles8/C and class unresolved; H-RETURN
	virtual void QueueRenderable(void* renderable, Gfx_Viewport* viewport, BYTE flag, DWORD value8,
		DWORD valueC) = 0;
	virtual void SetLighting(BYTE enabled) = 0; //60,+F0,5BDC90; raw BYTE -> D3DRS_LIGHTING137; H-RETURN
	virtual BYTE Lighting() = 0; //61,+F4,5C01F0; raw engine+2190
	//62,+F8,5C0220; D3DRS_AMBIENT139; H-RETURN
	virtual void SetAmbientRGB(BYTE red, BYTE green, BYTE blue) = 0;
	//63,+FC,5C0200; input reference versus pointer H-TYPE; H-RETURN
	virtual void SetAmbientColor(const D3DCOLOR* color) = 0;
	//64,+100,5C06B0; outputs1 byte each; H-RETURN
	virtual void GetAmbientRGB(BYTE* red, BYTE* green, BYTE* blue) = 0;
	//65,+104,5C0260; output reference versus pointer H-TYPE; H-RETURN
	virtual void GetAmbientColor(D3DCOLOR* color) = 0;
	//66,+108,5C06F0; viewport+360 -> D3DRS_FOGENABLE; H-RETURN
	virtual void SetFogEnabled(BYTE enabled, Gfx_Viewport* viewport) = 0;
	virtual BYTE FogEnabled(Gfx_Viewport* viewport) = 0; //67,+10C,5C0720
	//68,+110,5C0740; viewport+364; H-RETURN
	virtual void SetFogColor(const D3DCOLOR* color, Gfx_Viewport* viewport) = 0;
	virtual D3DCOLOR FogColor(Gfx_Viewport* viewport) = 0; //69,+114,5C0770
	//70,+118,5C0790; -> D3DRS_FOGSTART/FOGEND; H-RETURN
	virtual void SetFogRange(float start, float end, Gfx_Viewport* viewport) = 0;
	virtual void GetFogRange(float* start, float* end, Gfx_Viewport* viewport) = 0; //71,+11C,5C07C0; H-RETURN
	//72,+120,5C07F0; FLD dword: float/double source return H-TYPE
	virtual float FogDensity(Gfx_Viewport* viewport) = 0;
	//73,+124,5C0810; -> D3DRS_FOGDENSITY; H-RETURN
	virtual void SetFogDensity(float density, Gfx_Viewport* viewport) = 0;
	//74,+128,5BE010;40 stack bytes, no EDI input; unknown words may be float/enum H-TYPE; H-RETURN
	virtual void CreateHeightFog(D3DCOLOR color, float baseHeight, DWORD value8, DWORD valueC,
		float totalHeight, DWORD value14, DWORD value18, DWORD renderValue, float maxDensity,
		Gfx_Viewport* viewport) = 0;
	virtual void DestroyHeightFog(Gfx_Viewport* viewport) = 0; //75,+12C,5BE060; H-RETURN
	//76,+130,5BE080; actual0/1 success; raw stored byte
	virtual BYTE EnableHeightFog(BYTE enabled, Gfx_Viewport* viewport) = 0;
	virtual BYTE HeightFogEnabled(Gfx_Viewport* viewport) = 0; //77,+134,5BE0A0
	//78,+138,5BE0D0; renderer+72,engine+2193; semantic name inference; H-RETURN
	virtual void SetFogOfWarEnabled(BYTE enabled, Gfx_Viewport* viewport) = 0;
	virtual BYTE FogOfWarEnabled(Gfx_Viewport* viewport) = 0; //79,+13C,5BE100; same renderer+72
	virtual void SetFlag2194(BYTE enabled) = 0; //80,+140,5C0840; exact flag role unresolved; H-RETURN
	virtual BYTE Flag2194() = 0; //81,+144,5C0850
	//82,+148,5BFC50; renderer+60; null applies default+all listed viewports; H-RETURN
	virtual void SetRendererColor(D3DCOLOR color, Gfx_Viewport* viewport) = 0;
	virtual D3DCOLOR RendererColor(Gfx_Viewport* viewport) = 0; //83,+14C,5BDE60
	//84,+150,5C0280; rejects nonpositive/unordered, source return unresolved H-RETURN
	virtual void SetPositiveParameter84(float value) = 0;
	virtual float Parameter84() = 0; //85,+154,5C02A0; float/double source return H-TYPE
	virtual void SetPositiveParameter88(float value) = 0; //86,+158,5C02B0; H-RETURN
	virtual float Parameter88() = 0; //87,+15C,5C02D0; float/double source return H-TYPE
	virtual void ResetParameters84And88() = 0; //88,+160,5BDE90; defaults0.15/1.5; H-RETURN
	//89,+164,5BDEB0; actual Graphics caller supplies scene-render owner; exact class unresolved; H-RETURN
	virtual void SetRendererOwner76(void* owner, Gfx_Viewport* viewport) = 0;
	//90,+168,5BDEE0; validated via borrowed+D8; H-RETURN
	virtual void SetProjectedShadowTexture(DWORD handle) = 0;
	//91,+16C,5BDF80; alloc12*count, clears live count; H-RETURN
	virtual void SetProjectedShadowPointCapacity(UINT count) = 0;
	virtual UINT ProjectedShadowPointCapacity() = 0; //92,+170,5C02E0
	//93,+174,5BDFC0; reads3 float-width words; vector class/ref spelling H-TYPE; H-RETURN
	virtual void AddProjectedShadowPoint(const float* xyz) = 0;
	//94,+178,5C02F0; ANSI filename from5816A0; IDA WCHAR* is incorrect
	virtual HRESULT ScreenCapture(const char* filename) = 0;
	//95,+17C,5C1600; two vector pointers,8 stack bytes; no EBX/EBP/EDI/ESI inputs; H-RETURN/vector H-TYPE
	virtual void SetLevelAABB(const float* minimumXYZ, const float* maximumXYZ) = 0;
	//96,+180,5C0300; H-RETURN/vector H-TYPE
	virtual void GetLevelAABB(float* minimumXYZ, float* maximumXYZ) = 0;
	//97,+184,5BE240; H-RETURN/vector/ref H-TYPE
	virtual void CalculateMaskDimensions(const float* minimumXYZ, const float* maximumXYZ, UINT* width,
		UINT* height) = 0;
	//98,+188,5C0E10; only the low BYTE of exactSize is tested; source spelling H-TYPE; H-RETURN
	virtual void ConfigureAutomap(UINT width, UINT height, RECT* outputRect, const char* name,
		DWORD exactSize) = 0;
	virtual DWORD Value164() = 0; //99,+18C,5C0340; word role unresolved
	virtual BYTE AutomapReady() = 0; //100,+190,5C0350; actual creator sets engine+104
	virtual Gfx_Texture* AutomapMaskTexture() = 0; //101,+194,5C0360; borrowed+D8(engine+212)
	virtual Gfx_Texture* VisibleMaskTexture() = 0; //102,+198,5C0370; borrowed+D8(engine+216)
	//103,+19C,5C0380; constructor/SDK owner proof; bogus Concurrency FLIRT name rejected
	virtual IDirect3DTexture8* FogOfWarRenderTexture() = 0;
	//104,+1A0,5BEB40; clamp/convert -> opaque grayscale ARGB; role unresolved; H-RETURN
	virtual void SetGrayColor204(float intensity) = 0;
	virtual void SetGrayColor208(float intensity) = 0; //105,+1A4,5BEBB0; same for engine+208; H-RETURN
	//106,+1A8,5C0390; actual D3D8 CreateTexture owner proof; bogus FLIRT name rejected
	virtual IDirect3DTexture8* ProjectedShadowRenderTexture() = 0;
	//107,+1AC,5C03A0; engine+224/+228; H-RETURN
	virtual void GetProjectedShadowDimensions(UINT* width, UINT* height) = 0;
	//108,+1B0,5C0C00; strlen then real string assign at+F8 (248 decimal), statistics map name; H-RETURN
	virtual void SetString248(const char* value) = 0;
	//109,+1B4,5BDF30; validates via borrowed+D8; H-RETURN
	virtual void SetSilhouetteEdgeTexture(DWORD handle) = 0;
	//110,+1B8,5C03C0; raw handle+244, not a Concurrency scheduler pointer
	virtual DWORD SilhouetteEdgeTexture() = 0;
	//111,+1BC,5C03D0; actual Graphics config flag+74; exact role unresolved; H-RETURN
	virtual void SetFlag2188(BYTE value) = 0;
	//112,+1C0,5BDE20; ALPHATESTENABLE15 + ALPHAFUNC25 GREATER5/ALWAYS8; H-RETURN
	virtual void SetAlphaTest(BYTE enabled) = 0;
	virtual void SetWireframe(BYTE enabled) = 0; //113,+1C4,5C03E0; FILLMODE8 WIREFRAME2/SOLID3; H-RETURN
	//114,+1C8,5C0410; actual+1CC consumers gate light debug; H-RETURN
	virtual void SetDebugLightsEnabled(BYTE value) = 0;
	virtual BYTE DebugLightsEnabled() = 0; //115,+1CC,5C0420
	//116,+1D0,5C0640; renderer493430, source base/class name H-TYPE; H-RETURN
	virtual void SubmitLight(Gfx_Light* light, Gfx_Viewport* viewport) = 0;
	//117,+1D4,5C0680; flag role unresolved; H-RETURN
	virtual void SetRendererFlag116(BYTE value, Gfx_Viewport* viewport) = 0;
	//118,+1D8,5BEC20; writes full68-byte output record; original struct name unknown; H-RETURN
	virtual void GetVRAMStatistics(DWORD* output17Words) = 0;
	//119,+1DC,5C0430; no stack args; actual diagnostic owner51F6D0; H-RETURN
	virtual void PrintTexturePools() = 0;
};

//Original pointer variable0x707CE4, not Settings. Normal linker storage starts
//NULL; actual base construction publishes before Engine member construction.
extern Gfx_System_Interface* g_GfxSystem;

inline Gfx_System_Interface::Gfx_System_Interface()
{
	g_GfxSystem = this;
}
