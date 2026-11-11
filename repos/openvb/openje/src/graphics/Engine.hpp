#pragma once

#include "SystemInterface.hpp"
#include "EngineFrameState.hpp"
#include "GraphicsProjectedShadows.hpp"
#include <d3dfont.h>
#include <d3dx8math.h>
#include <list>
#include <string>

class Gfx_TextureManager;
class Gfx_BufferManager;
class Gfx_ModelDataManager;
class Gfx_SkinMeshManager;

// Actual engine identity with normal owned VC7 containers and the unmodified
// SDK CD3DFont. Offsets describe the observed x86 owner, not an allocation or
// serialization contract. All interface entries now have source bodies; the
// debug-light sphere child remains an explicit failure. Production startup
// and the preview still do not allocate this owner pending native integration.
class Gfx_Engine : public Gfx_System_Interface
{
public:
	Gfx_Engine(); //5BFE10; actual four managers, strings/list and SDK font.
	virtual ~Gfx_Engine(); //5C1CD0; shutdown before managers and reverse members.
	virtual void ShutdownDevice(); //5C1B30; actual resources/style/font/device order, unused EAX.
	virtual void ResetParameters84And88(); //5BDE90; actual0.15/1.5 defaults.
	virtual void SetProjectedShadowPointCapacity(UINT count); //5BDF80; uninitialized12-byte records.
	virtual void RenderFrame(); // 0x5C1940; full update policy, interface slot35/+8C.
	virtual void AdvanceTime(DWORD milliseconds); //5BDDB0, including actual water sampling51DA40.
	virtual void SetTimeMilliseconds(DWORD milliseconds); //5BDDF0; delta unchanged.
	virtual DWORD DeltaMilliseconds(); //5C0150.
	virtual DWORD TimeMilliseconds(); //5C0160.
	virtual float TimeSeconds(); //5C0170.
	bool RenderFrame(BYTE clearQueues, BYTE present); // 0x5BFCB0; distinct arguments, RET8.
	virtual Gfx_Viewport* DefaultViewport(); // 0x5C0180.
	virtual Gfx_Viewport* CurrentViewport(); // 0x5C0190.
	virtual Gfx_Viewport* PostViewport(); // 0x5C01A0.
	virtual void AddViewport(Gfx_Viewport* viewport); // 0x5C1B00; borrowed list payload.
	virtual Gfx_Texture* LookupTexture(DWORD handle); // 0x5C0930, slot54/+D8; borrowed result.
	virtual DWORD DeleteTexture(DWORD handle); //5C0910; actual published-manager release wrapper.
	virtual HRESULT CreateAndAddTexture(DWORD* output, UINT width, UINT height,
		UINT mipLevels, D3DFORMAT format, D3DPOOL pool); //5C08E0, tail521DB0, RET24.
	virtual HRESULT AddTexture(DWORD* output, Gfx_Texture* texture); //5C08F0, RET8.
	virtual BYTE CopyTextureRect(DWORD destinationHandle, UINT destinationMip,
		const RECT* destinationRect, DWORD sourceHandle, UINT sourceMip,
		const RECT* sourceRect); //5C0920, tail51F1A0, RET24.
	virtual BYTE AcquireTextureByName(DWORD* output, std::string name); //5C0970, RET32.
	virtual HRESULT LoadTextureFromFile(DWORD* output, std::string name,
		D3DFORMAT format, D3DPOOL pool, UINT mipLevels, DWORD filter,
		DWORD mipFilter, DWORD usage); //5C0A00, RET56.
	virtual HRESULT LoadTextureFromBuffer(DWORD* output, std::string name,
		const BYTE* data, UINT size, D3DFORMAT format, D3DPOOL pool, UINT mipLevels,
		DWORD filter, DWORD mipFilter, DWORD usage); //5C0A90, RET64.
	virtual void QueueMeshInstances(Gfx_MeshInstance* instances, Gfx_Viewport* viewport,
		UINT count); // 0x5C05B0, actual interface slot58/+E8; void source hypothesis.
	void RenderStatistics(); // 0x5BF9E0; actual SDK font overlay, unused result.
	void ProcessFogOfWar(); // 0x5BE380; actual owner fields, ECX only.
	void ProjectShadows(); // 0x5BEF80; actual owner fields, ECX only.
	void RenderAutomap(); // 0x5C1000; actual owner fields, ECX only.
	void ReleaseVideoResources(); // 0x5BEDF0; actual Engine receiver, not D3D.
	void RestoreVideoResources(); // 0x5BEF20; actual owners, tail-calls InitializeSystem.

	virtual DWORD AdapterCount(); //slot1,5C0440.
	virtual DWORD DeviceCount(UINT adapter); //slot2,5C0450.
	virtual DWORD ModeCount(UINT adapter, UINT device); //slot3,5C0480.
	virtual Gfx_D3DMode* Modes(UINT adapter, UINT device); //slot4,5C04D0.
	virtual BYTE VerticalSync(); //slot5,5C0080.
	virtual BYTE State68(); //slot7,5C0090.
	virtual bool FindMode(DWORD& output, UINT adapter, UINT device, DWORD width,
		DWORD height, BYTE color32, DWORD refresh); //slot10,5C05A0.
	virtual Gfx_D3DMode* CurrentMode(); //slot13,5C0520.
	virtual BYTE Windowed(); //slot14,5C00A0.
	virtual void SetColor(BYTE red, BYTE green, BYTE blue); //slot15,5C0860.
	virtual void SetViewportFlags(DWORD flags); //slot16,5C00B0.
	virtual DWORD ModeIndex(); //slot17,5C00C0; actual lower+A4, tentative name.
	virtual HWND Window(); //slot18,5C00D0.
	virtual DWORD ActiveLightLimit(); //slot19,5C00E0.
	virtual DWORD MaximumActiveLights(); //slot20,5C00F0.
	virtual DWORD SetActiveLightLimit(DWORD limit); //slot21,5C0530.
	virtual DWORD MaximumVertexBlendMatrices(); //slot22,5C0100.
	virtual void MaximumTextureSize(DWORD& width, DWORD& height); //slot23,5C0560.
	virtual void SetRendererFlag56(BYTE value); //slot24,5C0110.
	virtual BYTE RendererFlag56(); //slot25,5C0130.
	virtual void SetCamera(Gfx_Camera* camera); //slot40,5C01B0.
	virtual void SetCameraController(Gfx_CamCtrl* control); //slot41,5C1B20.
	virtual Gfx_Camera* Camera(); //slot42,5C01D0.
	virtual Gfx_CamCtrl* CameraController(); //slot43,5C01E0.
	virtual void SetPositiveParameter84(float value); //slot84,5C0280.
	virtual float Parameter84(); //slot85,5C02A0.
	virtual void SetPositiveParameter88(float value); //slot86,5C02B0.
	virtual float Parameter88(); //slot87,5C02D0.
	virtual UINT ProjectedShadowPointCapacity(); //slot92,5C02E0.
	virtual void EnableProjectedShadow(BYTE value); //26,5BDB20.
	virtual BYTE ProjectedShadowEnabled(); //27,5C0140.
	virtual void SetLinearFiltering(BYTE enabled); //28,5BDB70.
	virtual void SetMipFiltering(BYTE enabled); //29,5BDC10.
	virtual void SetLighting(BYTE enabled); //60,5BDC90.
	virtual BYTE Lighting(); //61,5C01F0.
	virtual void SetAmbientRGB(BYTE red, BYTE green, BYTE blue); //62,5C0220.
	virtual void SetAmbientColor(const D3DCOLOR* color); //63,5C0200.
	virtual void GetAmbientRGB(BYTE* red, BYTE* green, BYTE* blue); //64,5C06B0.
	virtual void GetAmbientColor(D3DCOLOR* color); //65,5C0260.
	virtual void SetFogEnabled(BYTE enabled, Gfx_Viewport* viewport); //66,5C06F0.
	virtual BYTE FogEnabled(Gfx_Viewport* viewport); //67,5C0720.
	virtual void SetFogColor(const D3DCOLOR* color, Gfx_Viewport* viewport); //68,5C0740.
	virtual D3DCOLOR FogColor(Gfx_Viewport* viewport); //69,5C0770.
	virtual void SetFogRange(float start, float end, Gfx_Viewport* viewport); //70,5C0790.
	virtual void GetFogRange(float* start, float* end, Gfx_Viewport* viewport); //71,5C07C0.
	virtual float FogDensity(Gfx_Viewport* viewport); //72,5C07F0.
	virtual void SetFogDensity(float density, Gfx_Viewport* viewport); //73,5C0810.
	virtual void CreateHeightFog(D3DCOLOR color, float baseHeight, DWORD value8,
		DWORD valueC, float totalHeight, DWORD value14, DWORD value18,
		DWORD renderValue, float maxDensity, Gfx_Viewport* viewport); //74,5BE010.
	virtual void DestroyHeightFog(Gfx_Viewport* viewport); //75,5BE060.
	virtual BYTE EnableHeightFog(BYTE enabled, Gfx_Viewport* viewport); //76,5BE080.
	virtual BYTE HeightFogEnabled(Gfx_Viewport* viewport); //77,5BE0A0.
	virtual void SetFogOfWarEnabled(BYTE enabled, Gfx_Viewport* viewport); //78,5BE0D0.
	virtual BYTE FogOfWarEnabled(Gfx_Viewport* viewport); //79,5BE100.
	virtual void SetFlag2194(BYTE enabled); //80,5C0840.
	virtual BYTE Flag2194(); //81,5C0850.
	virtual void SetRendererColor(D3DCOLOR color, Gfx_Viewport* viewport); //82,5BFC50.
	virtual D3DCOLOR RendererColor(Gfx_Viewport* viewport); //83,5BDE60.
	virtual void SetRendererOwner76(void* owner, Gfx_Viewport* viewport); //89,5BDEB0.
	virtual void SetProjectedShadowTexture(DWORD handle); //90,5BDEE0.
	virtual void AddProjectedShadowPoint(const float* xyz); //93,5BDFC0.
	virtual void SetSilhouetteEdgeTexture(DWORD handle); //109,5BDF30.
	virtual DWORD SilhouetteEdgeTexture(); //110,5C03C0.
	virtual void SetFlag2188(BYTE value); //111,5C03D0.
	virtual void SetAlphaTest(BYTE enabled); //112,5BDE20.
	virtual void SetWireframe(BYTE enabled); //113,5C03E0.
	virtual void SetDebugLightsEnabled(BYTE value); //114,5C0410.
	virtual BYTE DebugLightsEnabled(); //115,5C0420.
	virtual void SubmitLight(Gfx_Light* light, Gfx_Viewport* viewport); //116,5C0640.
	virtual void SetRendererFlag116(BYTE value, Gfx_Viewport* viewport); //117,5C0680.
	virtual DWORD RegisterModelData(Gfx_ModelData526620* object); //44,5C08A0.
	virtual DWORD ModelDataReferences(DWORD* handle); //45,5C08B0.
	virtual DWORD ReleaseModelData(DWORD* handle); //46,5C08C0.
	virtual Gfx_ModelInstance* CreateModelInstance(DWORD handle); //47,5C08D0.
	virtual bool CreateGeometryData(Gfx_MeshData* data); //56,5C0940.
	virtual void ReleaseGeometryData(Gfx_MeshData* data); //57,5C0950.
	virtual void QueueRenderable(void* renderable, Gfx_Viewport* viewport,
		BYTE flag, DWORD value8, DWORD valueC); //59,5C05F0.
	virtual HRESULT ScreenCapture(const char* filename); //94,5C02F0->502950.
	virtual void SetLevelAABB(const float* minimumXYZ, const float* maximumXYZ); //95,5C1600.
	virtual void GetLevelAABB(float* minimumXYZ, float* maximumXYZ); //96,5C0300.
	virtual void CalculateMaskDimensions(const float* minimumXYZ, const float* maximumXYZ,
		UINT* width, UINT* height); //97,5BE240.
	virtual void ConfigureAutomap(UINT width, UINT height, RECT* outputRect, const char* name,
		DWORD exactSize); //98,5C0E10; branch tests low BYTE, not the whole DWORD.
	virtual DWORD Value164(); //99,5C0340; automap output texture handle.
	virtual BYTE AutomapReady(); //100,5C0350; actual requested+68, not complete+69.
	virtual Gfx_Texture* AutomapMaskTexture(); //101,5C0360.
	virtual Gfx_Texture* VisibleMaskTexture(); //102,5C0370.
	virtual IDirect3DTexture8* FogOfWarRenderTexture(); //103,5C0380.
	virtual void SetGrayColor204(float intensity); //104,5BEB40.
	virtual void SetGrayColor208(float intensity); //105,5BEBB0.
	virtual IDirect3DTexture8* ProjectedShadowRenderTexture(); //106,5C0390.
	virtual void GetProjectedShadowDimensions(UINT* width, UINT* height); //107,5C03A0.
	virtual void SetString248(const char* value); //108,5C0C00.
	virtual void PrintTexturePools(); //119,5C0430->51F6D0.

	void CreateAutomapRenderTarget(); //5BE160; actual helper target, not5BE140.
	void CreateFogOfWarRenderTarget(); //5BE300.
	void CreateProjectedShadowRenderTarget(); //5BEAC0.
	virtual void SetVerticalSync(BYTE value); //5BF770.
	virtual BYTE CreateDevice(HWND owner, UINT adapter, UINT device, UINT mode,
		BYTE windowed, BYTE flag); //5C0C30.
	virtual void SetMode(DWORD mode); //5BF7E0.
	virtual void SetWindowMode(DWORD width, DWORD height, BYTE windowed, BYTE color32, DWORD refresh); //5BF8C0.
	BYTE PostDevice(); //5C0B40.
	void InitializeFog(); //5BDA60; proprietary prefix and actual SDK font restore.
	void InitializeSystem(); //5BDCB0; actual format-selection policy.
	virtual void GetVRAMStatistics(DWORD* output17Words); //5BEC20; original68-byte record.

	DWORD value04; // +04; raw lost-device state, only exact1 selects recovery.
	DWORD value08, value0C; //DeltaMS,totalMS.
	float value10; //+10; accumulated seconds, read directly by5C0170.
	Gfx_Viewport* currentViewport; // +14; getter0x5C0190.
	Gfx_Viewport* defaultViewport; // +18; getter0x5C0180 and the null-target branch.
	Gfx_Viewport* postViewport; // +1C; getter0x5C01A0.
	std::list<Gfx_Viewport*> additionalViewports; // +20..2B; owns nodes, borrows viewports.
	Gfx_TextureManager* textureManager; //+2C; owned,5211A0/520FC0.
	Gfx_BufferManager* bufferManager; //+30; owned,507810/507580.
	DWORD value34; //+34; constructor-unwritten, concrete role unresolved.
	Gfx_ModelDataManager* modelDataManager; //+38; owned,5CB510/5CB5C0.
	Gfx_SkinMeshManager* skinManager; //+3C; owned,50B6B0/50B5E0.
	HWND window; // +40; actual Present override window, not g_D3D.window.
	BYTE value44; //+44; initialized0, padding45..47 remains unwritten.
	void (__cdecl* lostVideoCallback)(); //+48; optional no-argument callback5BEDF0.
	void (__cdecl* restoredVideoCallback)(); //+4C; optional callback5BEF20.
	DWORD activeLightLimit; //+50, initialized0.
	float parameter84, parameter88; //+54/+58; reset5BDE90 supplies defaults.
	UINT shadowPointCapacity, shadowPointCount; //+5C/+60.
	JE::GraphicsProjectedShadows::Point* shadowPoints; //+64; owns raw POD array storage.
	BYTE automapRequested, automapReady; // +68/+69; post-automap writes0/1.
	UINT automapWidth, automapHeight; //+6C/+70; padding6A..6B unwritten.
	D3DXVECTOR3 levelMinimum, levelMaximum; //+74/+80.
	float levelWidth, levelHeight; //+8C/+90.
	UINT maskWidth, maskHeight; //+94/+98.
	IDirect3DSurface8* automapRenderTarget; //+9C; owned COM reference.
	IDirect3DSurface8* automapDepthSurface; //+A0; owned COM reference.
	DWORD automapTexture; //+A4; released by TextureManager.Clear, not a separate shutdown DeleteTexture.
	D3DFORMAT colorFormat, depthFormat; //+A8/+AC; ctor writes0.
	std::string automapTextureName; //+B0; actual normal empty-string lifetime.
	DWORD exploredGray, visibleGray; //+CC/+D0, FF808080/FFFFFFFF.
	DWORD automapMaskHandle, visibleMaskHandle; //+D4/+D8; owning texture references.
	IDirect3DTexture8* fogOfWarTarget; //+DC; owned COM reference.
	UINT shadowWidth, shadowHeight; //+E0/+E4.
	BYTE projectedShadows; // +E8; raw flag selecting the actual producer.
	DWORD projectedShadowTexture; //+EC; E9..EB remains unwritten padding.
	IDirect3DTexture8* projectedShadowTarget; //+F0; owned COM reference.
	DWORD textureF4; //+F4; owning handle, exact producer role unresolved.
	std::string mapName; //+F8; actual normal string lifetime.
	CD3DFont statisticsFont; //+114..78B; supplied unmodified ANSI SDK class, Arial12/flags0.
	char statisticsText[256]; //+78C..88B; constructor leaves contents unwritten.
	BYTE statistics; // +88C; selects the actual unresolved overlay.
	BYTE transparency, lighting, value88F, value890; //+88D..890; initialized0.
	BYTE fogOfWar; // +891; raw producer flag.
	BYTE revealExplored; //+892; initialized1, padding893 remains unwritten.
	// These four owners are consumed only through the actual COM Release slot
	// in this lifetime family; concrete producer interfaces remain unresolved.
	IUnknown* resource894;
	IUnknown* resource898;
	IUnknown* resource89C;
	IUnknown* resource8A0;

private:
	Gfx_EngineFrameView FrameView(); //Live field aliases for shared frame policies.
	Gfx_Engine(const Gfx_Engine&);
	Gfx_Engine& operator=(const Gfx_Engine&);
};

extern BYTE g_GfxDebugFrames; //Original70C968, canonical raw global in EngineFrame.cpp.
