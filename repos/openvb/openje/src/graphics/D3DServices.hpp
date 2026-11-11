#pragma once

#include "D3DContext.hpp"

namespace JE { namespace Gfx { namespace D3DServices
{
	IDirect3D8* Create(UINT version);
	BOOL PerformanceFrequency(LARGE_INTEGER* output);
	UINT AdapterCount(IDirect3D8* d3d);
	HRESULT AdapterDisplayMode(IDirect3D8* d3d, UINT adapter, D3DDISPLAYMODE* output);
	HRESULT AdapterIdentifier(IDirect3D8* d3d, UINT adapter, D3DADAPTER_IDENTIFIER8* output);
	UINT AdapterModeCount(IDirect3D8* d3d, UINT adapter);
	HRESULT AdapterMode(IDirect3D8* d3d, UINT adapter, UINT mode, D3DDISPLAYMODE* output);
	HRESULT DeviceCaps(IDirect3D8* d3d, UINT adapter, D3DDEVTYPE type, D3DCAPS8* output);
	HRESULT CheckDeviceType(IDirect3D8* d3d, UINT adapter, D3DDEVTYPE type,
		D3DFORMAT display, D3DFORMAT backBuffer, BOOL windowed);
	HRESULT CheckDeviceFormat(IDirect3D8* d3d, UINT adapter, D3DDEVTYPE type,
		D3DFORMAT display, DWORD usage, D3DRESOURCETYPE resource, D3DFORMAT format);
	HRESULT CheckDepthStencilMatch(IDirect3D8* d3d, UINT adapter, D3DDEVTYPE type,
		D3DFORMAT display, D3DFORMAT backBuffer, D3DFORMAT depth);
	BOOL ClientRect(HWND owner, RECT* output);
	int SystemMetrics(int index);
	BOOL Move(HWND owner, int x, int y, int width, int height, BOOL repaint);
	HRESULT CreateDevice(IDirect3D8* d3d, UINT adapter, D3DDEVTYPE type, HWND owner,
		DWORD behavior, D3DPRESENT_PARAMETERS* parameters, IDirect3DDevice8** output);
	UINT AvailableTextureMemory(IDirect3DDevice8* device);
	HRESULT RenderState(IDirect3DDevice8* device, D3DRENDERSTATETYPE state, DWORD value);
	HRESULT TextureStageState(IDirect3DDevice8* device, DWORD stage,
		D3DTEXTURESTAGESTATETYPE state, DWORD value);
	HRESULT Transform(IDirect3DDevice8* device, D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix);
	HRESULT Texture(IDirect3DDevice8* device, DWORD stage, IDirect3DBaseTexture8* texture);
	ULONG Release(IDirect3DDevice8* device);
	ULONG Release(IDirect3D8* d3d);
}}}
