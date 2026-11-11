#include "D3DServices.hpp"

namespace JE { namespace Gfx { namespace D3DServices
{
	IDirect3D8* Create(UINT version) { return Direct3DCreate8(version); }
	BOOL PerformanceFrequency(LARGE_INTEGER* output) { return ::QueryPerformanceFrequency(output); }
	UINT AdapterCount(IDirect3D8* d3d) { return d3d->GetAdapterCount(); }
	HRESULT AdapterDisplayMode(IDirect3D8* d3d, UINT adapter, D3DDISPLAYMODE* output)
	{ return d3d->GetAdapterDisplayMode(adapter, output); }
	HRESULT AdapterIdentifier(IDirect3D8* d3d, UINT adapter, D3DADAPTER_IDENTIFIER8* output)
	{ return d3d->GetAdapterIdentifier(adapter, 0, output); }
	UINT AdapterModeCount(IDirect3D8* d3d, UINT adapter) { return d3d->GetAdapterModeCount(adapter); }
	HRESULT AdapterMode(IDirect3D8* d3d, UINT adapter, UINT mode, D3DDISPLAYMODE* output)
	{ return d3d->EnumAdapterModes(adapter, mode, output); }
	HRESULT DeviceCaps(IDirect3D8* d3d, UINT adapter, D3DDEVTYPE type, D3DCAPS8* output)
	{ return d3d->GetDeviceCaps(adapter, type, output); }
	HRESULT CheckDeviceType(IDirect3D8* d3d, UINT adapter, D3DDEVTYPE type,
		D3DFORMAT display, D3DFORMAT backBuffer, BOOL windowed)
	{ return d3d->CheckDeviceType(adapter, type, display, backBuffer, windowed); }
	HRESULT CheckDeviceFormat(IDirect3D8* d3d, UINT adapter, D3DDEVTYPE type,
		D3DFORMAT display, DWORD usage, D3DRESOURCETYPE resource, D3DFORMAT format)
	{ return d3d->CheckDeviceFormat(adapter, type, display, usage, resource, format); }
	HRESULT CheckDepthStencilMatch(IDirect3D8* d3d, UINT adapter, D3DDEVTYPE type,
		D3DFORMAT display, D3DFORMAT backBuffer, D3DFORMAT depth)
	{ return d3d->CheckDepthStencilMatch(adapter, type, display, backBuffer, depth); }
	BOOL ClientRect(HWND owner, RECT* output) { return ::GetClientRect(owner, output); }
	int SystemMetrics(int index) { return ::GetSystemMetrics(index); }
	BOOL Move(HWND owner, int x, int y, int width, int height, BOOL repaint)
	{ return ::MoveWindow(owner, x, y, width, height, repaint); }
	HRESULT CreateDevice(IDirect3D8* d3d, UINT adapter, D3DDEVTYPE type, HWND owner,
		DWORD behavior, D3DPRESENT_PARAMETERS* parameters, IDirect3DDevice8** output)
	{ return d3d->CreateDevice(adapter, type, owner, behavior, parameters, output); }
	UINT AvailableTextureMemory(IDirect3DDevice8* device) { return device->GetAvailableTextureMem(); }
	HRESULT RenderState(IDirect3DDevice8* device, D3DRENDERSTATETYPE state, DWORD value)
	{ return device->SetRenderState(state, value); }
	HRESULT TextureStageState(IDirect3DDevice8* device, DWORD stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
	{ return device->SetTextureStageState(stage, state, value); }
	HRESULT Transform(IDirect3DDevice8* device, D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix)
	{ return device->SetTransform(state, matrix); }
	HRESULT Texture(IDirect3DDevice8* device, DWORD stage, IDirect3DBaseTexture8* texture)
	{ return device->SetTexture(stage, texture); }
	ULONG Release(IDirect3DDevice8* device) { return device->Release(); }
	ULONG Release(IDirect3D8* d3d) { return d3d->Release(); }
}}}
