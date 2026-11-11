#pragma once

#include <windows.h>

class Gfx_System_Interface;
class Gfx_CamCtrl;
class Octree2Gfx;
class MapMaskManager;
class OCTREE;
class OCTREE_Camera;
class Gfx_Light;
struct Gfx_D3DMode;
namespace JE { namespace Input { struct InputEvent; } }

//Original global RTTI/vtable identity Graphics, not the historical upstream
//F3 namespace hypothesis. These are the actual frontend's sole member owners;
//configuration/scene factors are not embedded or copied into this object.
struct Graphics
{
public:
	Graphics(); //580E20.
	virtual ~Graphics(); //583850;5838B0 scalar deletion is ordinary virtual C++ lifetime.
	virtual void PrepareScene(BYTE* mapChanged); //5828D0; RET4, output byte written first.
	virtual void Present(); //5811C0.
	virtual void AttachCamera(Gfx_CamCtrl* control); //581210; borrowed controller.
	virtual BYTE ProcessInput(const JE::Input::InputEvent& event); //581230.
	virtual void ReadSettings(); //581C30; actual engine/main settings owners.
	virtual void WriteSettings(); //582BB0; writes main settings in memory.

	void Initialize(DWORD unused); //581810; RET4, argument unread by original.
	void Cleanup(); //581490; GUI cleanup then live global backend shutdown.
	void SetShadowMode(DWORD mode); //580F80.
	DWORD CycleShadowMode(); //580FF0.
	void SetLinearFiltering(BYTE value); //581620, raw55 then global backend.
	void SetWireframe(BYTE value); //581660, raw58 then global backend.
	void SetTransparency(BYTE value); //581680, raw59 then actual AlphaTest API.
	void SetMipFiltering(BYTE value); //581020, raw57 then member backend.
	void ToggleStatistics(); //Owned581640, raw4A then global backend.
	void ToggleFogOfWar(); //Owned581110, real nested backend getter/setter.
	BYTE GetFogOfWarEnabled(); //581140, actual backend/default viewport.
	BYTE GetFogEnabled(); //581040, actual member backend/default viewport.
	void SetLighting(BYTE value); //581600; publishes before the global backend call.
	void SetStartupMovies(BYTE value); //581010, raw BYTE publication.
	void SetFogOfWarEnabled(BYTE enabled); //5810F0, actual member backend and NULL viewport.
	void SetBuildingVisible(LONG building, BYTE visible); //581460; optional renderer.
	BYTE GetBuildingVisible(LONG building); //581470; missing renderer returns1.
	void SetDebugLights(BYTE value); //581150; member backend, raw BYTE.
	void SetDebugOctreeLights(BYTE value); //581170; renderer+80 publication.
	void ToggleHeightFog(); //Shared5810B0 tail of588340.
	void QueueExploredChange(DWORD mapID, LONG column, LONG row, BYTE value); //581310.
	void QueueVisibleChange(DWORD entityID, DWORD mapID, LONG column, LONG row, BYTE value); //581320.
	void ClearMapMasks(); //581330, optional actual maskManager clear.
	Gfx_Light* GetStaticLights(DWORD* count); //5811A0; missing renderer preserves count.
	void GetVRAMStatistics(DWORD* output17Words); //581340; full68-byte record.
	void PrintTexturePools(); //581390, optional backend virtual+1DC.
	BYTE GetVerticalSync(); //5813A0; refreshes stored value from a present backend.
	void SetVerticalSync(BYTE value); //5813C0; stored before optional backend call.
	DWORD GetModeCount(); //5813E0.
	Gfx_D3DMode* GetModes(); //581400.
	Gfx_D3DMode* GetCurrentMode(); //581420.
	const char* GetAdapterDescription(); //581430; actual D3D adapter array.
	BOOL ToggleFullscreen(); //5814B0; actual Win32 window and current global backend.
	void GetScreenDimensions(LONG& width, LONG& height); //581590; actual global window/backend, RET8.
	BYTE SelectDisplayMode(DWORD mode); //5817A0; unsigned range and original field order.
	void CaptureScreen(); //5816A0; body uses global backend, shared caller loads this.
	void SetFogEnabled(BYTE enabled); //581050.
	void SetFogColor(DWORD color); //581070; actual full ARGB argument.
	void SetFogRange(float start, float end); //581090.
	OCTREE_Camera* GetPickingCamera(); //581190; borrowed renderer camera, NULL without renderer.

	BYTE initialized; //+04; ordinary padding05..07 stays unwritten.
	Gfx_System_Interface* backend; //+08; factory-acquired singleton, released by destructor.
	Gfx_CamCtrl* cameraControl; //+0C; borrowed, never deleted by Graphics.
	Octree2Gfx* renderer; //+10; owns actual renderer and its normal containers.
	MapMaskManager* maskManager; //+14; actual owned virtual object.
	BYTE liveFullscreen, liveColor32; //+18/+19; ctor1/0, padding1A..1B unwritten.
	DWORD liveWidth, liveHeight, liveRefresh; //+1C/+20/+24; ctor800/600/0.
	DWORD value28; //+28; ctor-unwritten, role unresolved.
	DWORD adapter, device, displayMode; //+2C/+30/+34; ctor0.
	BYTE fullscreen; //+38; requested display state, ctor-unwritten; padding39..3B unwritten.
	DWORD width, height, refresh; //+3C/+40/+44; requested values, ctor-unwritten.
	BYTE color32; //+48; requested format, ctor-unwritten.
	BYTE vsync, statistics, clearRed, clearGreen, clearBlue; //+49..4D; ctor1/0/0/64/64.
	DWORD shadowMode; //+50; padding4E..4F unwritten, ctor mode0.
	BYTE lighting, linearFiltering, startupMovies, flag57; //+54..57; ctor1.
	BYTE wireframe, transparency, flag5A, flag5B; //+58..5B; ctor0/1/0/0.
	BYTE sceneEnabled, bounds, debugBounds; //+5C..5E; ctor1/0/0, padding5F unwritten.
	OCTREE* currentOctree; //+60; borrowed, ctorNULL.
	DWORD previousClock; //+64; actual56AF50 clock captured after renderer construction.

private:
	Graphics(const Graphics&);
	Graphics& operator=(const Graphics&);
};

Graphics* Gfx_CreateGraphics(); //587B70; actual frontend allocation/construction.
