#pragma once

#include <windows.h>
#include <d3d8.h>

#include <vector>

#include "MatrixStackBridge.hpp"

namespace JE { namespace Gfx
{
	struct DeviceMode
	{
		DeviceMode()
			: width(0), height(0), refreshRate(0), displayFormat(D3DFMT_UNKNOWN),
			backBufferFormat(D3DFMT_UNKNOWN), depthStencilFormat(D3DFMT_UNKNOWN),
			behavior(0), stencil(0) {}
		UINT width, height, refreshRate;
		D3DFORMAT displayFormat;
		D3DFORMAT backBufferFormat;
		D3DFORMAT depthStencilFormat;
		DWORD behavior;
		BYTE stencil;
	};

	struct DeviceInfo
	{
		D3DDEVTYPE type;
		char description[4];
		D3DCAPS8 caps;
		BYTE windowedSupported, guardBandSupported, vertexShaderSupported, pixelShaderSupported;
		std::vector<DeviceMode> modes;
	};

	struct AdapterInfo
	{
		AdapterInfo() : windowDepthStencil(D3DFMT_UNKNOWN), windowBehaviorKnown(false) {}
		D3DADAPTER_IDENTIFIER8 identifier;
		std::vector<DeviceInfo> devices;
		D3DDISPLAYMODE desktop;
		D3DFORMAT windowDepthStencil;
		DWORD windowBehavior;
		bool windowBehaviorKnown;
	};

	struct TextureStageState
	{
		IDirect3DBaseTexture8* texture;
		DWORD values[29];
	};

	// Native owner for the recovered lower Gfx_D3D unit, not Gfx_Engine/Graphics.
	// COM lifetime follows explicit ReleaseDevice/Shutdown, as in the original.
	class D3DContext
	{
	public:
		D3DContext()
			: direct3D(NULL), device(NULL), matrixStack(NULL), adapterCount(0),
			currentAdapter(0), currentDeviceIndex(0), currentModeIndex(0),
			currentDevice(NULL), currentMode(NULL), requestedWindowed(0), ready(0),
			verticalSync(1), textureStageCount(0) {}
		void Initialize(BYTE allowReference = 0);
		void EnumerateDevices(BYTE allowReference);
		bool CheckDeviceRequirements(const DeviceInfo& info, UINT adapter,
			D3DFORMAT backBufferFormat, DWORD behavior);
		bool ChooseDepthStencil(D3DFORMAT& output, UINT adapter, D3DDEVTYPE type,
			D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat);
		bool FindMode(UINT adapter, UINT deviceIndex, UINT width, UINT height,
			UINT refreshRate, BYTE color32, UINT& output) const;
		HRESULT CreateDevice(HWND owner, UINT adapter, UINT deviceIndex, UINT mode,
			BYTE windowed, BYTE multithreaded);
		void InitializeRenderStates();
		UINT InitializeTextureStates();
		HRESULT ApplyDefaultRenderStates();
		UINT ApplyDefaultTextureStates();
		DWORD ApplyStates(BYTE resetDefaults);
		HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
		HRESULT SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix);
		HRESULT SetTexture(UINT stage, IDirect3DBaseTexture8* texture);
		HRESULT SetTextureStageState(UINT stage, D3DTEXTURESTAGESTATETYPE state, DWORD value);
		void ReleaseDevice();
		void Shutdown();

		IDirect3D8* direct3D;
		IDirect3DDevice8* device;
		HWND window;
		D3DPRESENT_PARAMETERS presentation;
		D3DMATRIX identity;
		MatrixStackBridge::Handle matrixStack;
		std::vector<AdapterInfo> adapters;
		UINT adapterCount, currentAdapter, currentDeviceIndex, currentModeIndex;
		DeviceInfo* currentDevice;
		DeviceMode* currentMode;
		DeviceMode windowMode;
		BYTE requestedWindowed, ready, verticalSync;
		DWORD field0xA4, field0xA8, field0xAC;
		// Original counters +176..239; transform calls are element10, texture calls8.
		DWORD statistics[16];
		DWORD defaultRenderStates[174], renderStates[174];
		UINT textureStageCount;
		std::vector<TextureStageState> defaultTextureStates, textureStates;
		LARGE_INTEGER performanceFrequency;
		float inverseFrequency;
		DWORD field0x6B4;
	};

	// Ordering from original qsort callback502510; uses the real standard sorter.
	bool DisplayModeLess(const D3DDISPLAYMODE& left, const D3DDISPLAYMODE& right);
}}
