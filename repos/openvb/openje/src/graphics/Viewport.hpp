#pragma once

#include <d3d8.h>
#include "Camera.hpp"
#include "MeshData.hpp"
#include "UIVertex.hpp"

class Gfx_RenderManager;
class Gfx_CamCtrl;

// Canonical identity for the actual nonvirtual viewport also called
// GUI::cls_0x4c0190. The original allocation is640 bytes. Scalar source names
// remain descriptive; the embedded members own their normal C++ lifetimes.
class Gfx_Viewport
{
public:
	Gfx_Viewport(); // 0x4C0190, including the original member cleanup graph.
	~Gfx_Viewport(); // 0x4296C0; ordinary destruction, not owner storage deletion.
	HRESULT SetViewport(UINT x, UINT y, UINT width, UINT height, float minimumZ, float maximumZ); //4248F0.
	void SetClearColor(BYTE red, BYTE green, BYTE blue, BYTE alpha); //424DB0.
	HRESULT SetViewport(const D3DVIEWPORT8& descriptor); // 0x424880; forwards renderer result.
	void RebuildQuad(); // 0x4BFFD0; borrowed embedded vertices, actual mesh ownership.
	void Render(BYTE clear); // 0x4BFF20; camera/fog/clear then live renderer; incidental AL unused.
	void SetCameraController(Gfx_CamCtrl* controller); // 0x429D20; borrowed owner/keys.

	D3DVIEWPORT8 viewport; // +00..17; actual six-word SDK layout copied by0x424880.
	float aspect; // +18.
	BYTE flag1C; // +1C; padding+1D..1F remains unwritten.
	DWORD clearFlags; // +20; source spelling hypothesis, initialized3.
	D3DCOLOR clearColor; // +24; initializedFF000100, also used by0x4BFFD0.
	float clearDepth; // +28.
	DWORD clearStencil; // +2C.
	Gfx_Camera camera; // +30..15B; actual embedded300-byte owner.
	Gfx_Camera* activeCamera; // +15C; actual DrawGroup0x490928, may reference embedded camera+30.
	Gfx_CamCtrl* cameraController; // +160; lifetime initializes0, destructor does not detach.
	Gfx_RenderManager* renderer; // +164; constructor0x4C02FB and wrapper0x5C05B0.
	BYTE fogEnabled; // +168; actual base material0x53BCF1 reads the raw byte.
	D3DCOLOR fogColor; // +16C; actual setter0x5C0740.
	float fogDensity, fogStart, fogEnd; // +170/+174/+178; actual5C0810/5C0790.
	JE::Gfx::UIVertex vertices[4]; // +17C..1EB; left unwritten by construction.
	Gfx_MeshData mesh; // +1EC..257;0x4BFFD0 later borrows the embedded vertices.
	Gfx_MeshInstance instance; // +258..27F; owns its normal material/flag state.

private:
	Gfx_Viewport(const Gfx_Viewport&);
	Gfx_Viewport& operator=(const Gfx_Viewport&);
};
