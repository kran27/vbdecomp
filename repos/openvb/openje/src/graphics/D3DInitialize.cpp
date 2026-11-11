// SDK framework baseline: Copyright (c) 1998-2001 Microsoft Corporation.
// Actual F3 policies/arrays differ; see the retained SDK source and function record.
#include "D3DState.hpp"
#include "D3DModeOrder.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <cstdlib>
#include <cstring>
#include <new>
#include <vector>

Gfx_D3D *Gfx_D3D::Initialize(BYTE allowReference)
{
	D3DXMatrixIdentity(&identity);
	windowMode.backBufferFormat = D3DFMT_UNKNOWN;
	windowMode.depthStencilFormat = D3DFMT_UNKNOWN;
	windowMode.behavior = 0;
	windowMode.stencil = 0;
	windowMode.display.Width = 0;
	windowMode.display.Height = 0;
	windowMode.display.Format = D3DFMT_UNKNOWN;
	windowMode.display.RefreshRate = 0;
	device = 0;
	std::memset(&presentation, 0, sizeof(presentation));
	matrixStack = 0;
	ready = 0;
	textureStageCount = 0;
	textureStates = 0; // Original leaves defaultTextureStates untouched.
	adapterCount = 0;
	adapters = 0;
	currentAdapter = currentDeviceIndex = 0;
	currentDevice = 0;
	currentModeIndex = 0;
	currentMode = 0;
	requestedWindowed = 0;
	QueryPerformanceFrequency(&performanceFrequency); // Return value is ignored.
	const double frequency = static_cast<double>(performanceFrequency.QuadPart);
	previousStatisticsTime = 0;
	statistics[0] = statistics[1] = 0;
	const float reciprocal = static_cast<float>(1.0 / frequency);
	statistics[2] = statistics[3] = 0;
	statistics[4] = statistics[5] = statistics[14] = statistics[15] = 0;
	statistics[6] = statistics[7] = statistics[8] = statistics[9] = 0;
	statistics[10] = statistics[11] = statistics[12] = statistics[13] = 0;
	verticalSync = 1;
	inverseFrequency = reciprocal;
	direct3D = Direct3DCreate8(D3D_SDK_VERSION);
	if (direct3D)
	{
		JE::LogDebugString("Created D3D\n");
		EnumerateDevices(allowReference);
		return this;
	}
	JE::FatalError("Failed to Create D3D!  Please ensure that DX8.1 is installed properly.\n");
	return this;
}

namespace
{
static bool CheckDeviceRequirements(Gfx_D3DDeviceInfo* info, IDirect3D8*,
	UINT adapter, const Gfx_D3DFormatInfo& format)
{
	if (info->type == D3DDEVTYPE_HAL)
	{
		bool accepted = true;
		if ((format.behavior & D3DCREATE_HARDWARE_VERTEXPROCESSING) && info->caps.MaxVertexBlendMatrices < 4)
		{
			accepted = false;
			JE::LogDebugString("[Enum] HWVP requires 4 blend matrices!\n");
		}
		if (info->caps.MaxVertexIndex < 65535)
		{
			accepted = false;
			JE::LogDebugString("[Enum] Device doesn't support minimum of 65536 indices!\n");
		}
		if (info->caps.MaxPrimitiveCount < 65535)
		{
			accepted = false;
			JE::LogDebugString("[Enum] Device doesn't support minimum of 65536 primitives !\n");
		}
		if (info->caps.MaxSimultaneousTextures < 2)
		{
			accepted = false;
			JE::LogDebugString("[Enum] Device doesn't support minimum of 2 simultaneous textures!\n");
		}
		if (info->caps.MaxTextureWidth < 1024 || info->caps.MaxTextureHeight < 1024)
		{
			accepted = false;
			JE::LogDebugString("[Enum] Device doesn't support minimum texture size of 1024x1024!\n");
		}
		if (!(info->caps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN))
		{
			accepted = false;
			JE::LogDebugString("[Enum] Device doesn't support TEXGEN!\n");
		}
		if (!(info->caps.TextureOpCaps & 0x105F))
		{
			accepted = false;
			JE::LogDebugString("[Enum] Device doesn't support required TextureOp!\n");
		}
		const D3DFORMAT formats[] = {D3DFMT_DXT1, D3DFMT_DXT2, D3DFMT_DXT3, D3DFMT_DXT4, D3DFMT_DXT5};
		for (unsigned i = 0; i < 5; ++i)
		{
			// Original uses the global interface, ignoring the passed interface.
			if (FAILED(g_D3D.direct3D->CheckDeviceFormat(adapter, info->type, format.backBuffer, 0,
														 D3DRTYPE_TEXTURE, formats[i])))
			{
				JE::LogDebugString("[Enum] Device doesn't support DXT texture format: %x\n",
								   formats[i]);
				return false;
			}
		}
		return accepted;
	}
	return true;
}
}

bool Gfx_D3D::ChooseDepthStencil(D3DFORMAT &output, UINT adapter, D3DDEVTYPE type,
								 D3DFORMAT adapterFormat, D3DFORMAT backBuffer, DWORD, DWORD)
{
	// Adapted SDK depth checks: original ignores both trailing arguments and
	// always tries this game-specific order, with no minimum-bit thresholds.
	if (SUCCEEDED(direct3D->CheckDeviceFormat(adapter, type, adapterFormat, D3DUSAGE_DEPTHSTENCIL,
											  D3DRTYPE_SURFACE, D3DFMT_D24S8)) &&
		SUCCEEDED(direct3D->CheckDepthStencilMatch(adapter, type, adapterFormat, backBuffer,
												   D3DFMT_D24S8)))
	{
		output = D3DFMT_D24S8;
		return true;
	}
	if (SUCCEEDED(direct3D->CheckDeviceFormat(adapter, type, adapterFormat, D3DUSAGE_DEPTHSTENCIL,
											  D3DRTYPE_SURFACE, D3DFMT_D24X4S4)) &&
		SUCCEEDED(direct3D->CheckDepthStencilMatch(adapter, type, adapterFormat, backBuffer,
												   D3DFMT_D24X4S4)))
	{
		output = D3DFMT_D24X4S4;
		return true;
	}
	if (SUCCEEDED(direct3D->CheckDeviceFormat(adapter, type, adapterFormat, D3DUSAGE_DEPTHSTENCIL,
											  D3DRTYPE_SURFACE, D3DFMT_D15S1)) &&
		SUCCEEDED(direct3D->CheckDepthStencilMatch(adapter, type, adapterFormat, backBuffer,
												   D3DFMT_D15S1)))
	{
		output = D3DFMT_D15S1;
		return true;
	}
	if (SUCCEEDED(direct3D->CheckDeviceFormat(adapter, type, adapterFormat, D3DUSAGE_DEPTHSTENCIL,
											  D3DRTYPE_SURFACE, D3DFMT_D32)) &&
		SUCCEEDED(
			direct3D->CheckDepthStencilMatch(adapter, type, adapterFormat, backBuffer, D3DFMT_D32)))
	{
		output = D3DFMT_D32;
		return true;
	}
	if (SUCCEEDED(direct3D->CheckDeviceFormat(adapter, type, adapterFormat, D3DUSAGE_DEPTHSTENCIL,
											  D3DRTYPE_SURFACE, D3DFMT_D24X8)) &&
		SUCCEEDED(direct3D->CheckDepthStencilMatch(adapter, type, adapterFormat, backBuffer,
												   D3DFMT_D24X8)))
	{
		output = D3DFMT_D24X8;
		return true;
	}
	if (SUCCEEDED(direct3D->CheckDeviceFormat(adapter, type, adapterFormat, D3DUSAGE_DEPTHSTENCIL,
											  D3DRTYPE_SURFACE, D3DFMT_D16)) &&
		SUCCEEDED(
			direct3D->CheckDepthStencilMatch(adapter, type, adapterFormat, backBuffer, D3DFMT_D16)))
	{
		output = D3DFMT_D16;
		return true;
	}
	output = D3DFMT_UNKNOWN;
	return false;
}

bool Gfx_D3D::FindMode(DWORD &output, UINT adapter, UINT deviceIndex, DWORD width, DWORD height,
					   BYTE color32, DWORD refreshRate)
{
	bool result = false;
	if (adapter < adapterCount)
	{
		Gfx_D3DAdapterInfo &adapterInfo = adapters[adapter];
		if (deviceIndex < adapterInfo.deviceCount)
		{
			Gfx_D3DDeviceInfo &info = adapterInfo.devices[deviceIndex];
			DWORD count = info.modeCount;
			Gfx_D3DMode *modes = info.modes;
			for (output = 0; output < count; ++output)
			{
				Gfx_D3DMode &mode = modes[output];
				if (mode.display.Width != width || mode.display.Height != height)
					continue;
				D3DFORMAT format = mode.backBufferFormat;
				if (color32)
				{
					if (format != D3DFMT_X8R8G8B8 && format != D3DFMT_A8R8G8B8)
						continue;
				}
				else if (format != D3DFMT_R5G6B5 && format != D3DFMT_X1R5G5B5 &&
						 format != D3DFMT_A1R5G5B5 && format != D3DFMT_A4R4G4B4 &&
						 format != D3DFMT_X4R4G4B4)
					continue;
				if (refreshRate)
				{
					if (mode.display.RefreshRate >= refreshRate)
						return true;
				}
				else if (mode.display.RefreshRate >= 60)
					return true;
			}
			return false;
		}
	}
	return result;
}

void Gfx_D3D::EnumerateDevices(BYTE allowReference)
{
	DWORD count = direct3D->GetAdapterCount();
	if (!count)
		JE::FatalError("No D3D Adapters found!  Please ensure that DX8.1 device drivers is "
					   "installed properly.\n");
	std::vector<Gfx_D3DFormatInfo> formats;
	formats.reserve(5);
	adapters = new Gfx_D3DAdapterInfo[count];
	DWORD compatible = 0;
	for (DWORD a = 0; a < count; ++a)
	{
		Gfx_D3DAdapterInfo *adapter = &adapters[compatible];
		direct3D->GetAdapterDisplayMode(a, &adapter->desktop); // HRESULT ignored.
		adapter->windowDepthStencil = D3DFMT_UNKNOWN;
		HRESULT result = direct3D->GetAdapterIdentifier(a, 0, &adapter->identifier);
		if (FAILED(result))
		{
			JE::LogDebugString("Gfx_D3D::EnumDevices - GetAdapterIdentifier() failed: Code: %x\n",
							   result);
			continue;
		}
		DWORD available = direct3D->GetAdapterModeCount(a);
		D3DDISPLAYMODE *modes = new D3DDISPLAYMODE[available];
		if (!modes)
			JE::FatalError("Out of memory - could not allocate pD3D_displaymodes\n");
		DWORD kept = 0;
		for (DWORD m = 0; m < available; ++m)
		{
			D3DDISPLAYMODE mode;
			result = direct3D->EnumAdapterModes(a, m, &mode);
			if (FAILED(result))
				JE::LogDebugString("Gfx_D3D::EnumDevices - EnumAdapterModes() failed! Code: %x\n",
								   result);
			else if (mode.Width >= 640 && mode.Height >= 480)
			{
				modes[kept++] = mode;
				DWORD f = 0;
				while (f < formats.size() && formats[f].display != mode.Format)
					++f;
				if (f == formats.size())
				{
					Gfx_D3DFormatInfo format; // Other bytes intentionally unwritten.
					format.display = format.backBuffer = mode.Format;
					formats.push_back(format);
				}
			}
		}
		std::qsort(modes, kept, sizeof(D3DDISPLAYMODE), CompareD3DDisplayModes);
		const D3DDEVTYPE types[] = {D3DDEVTYPE_HAL, D3DDEVTYPE_REF};
		DWORD typeCount = allowReference ? 2 : 1;
		adapter->deviceCount = 0;
		adapter->devices = new Gfx_D3DDeviceInfo[typeCount]; // POD payloads stay unwritten.
		DWORD devices = 0;
		for (DWORD t = 0; t < typeCount; ++t)
		{
			Gfx_D3DDeviceInfo *info = &adapter->devices[devices];
			result = direct3D->GetDeviceCaps(a, types[t], &info->caps);
			if (FAILED(result))
			{
				JE::LogDebugString(
					"Gfx_D3D::EnumDevices - Failed to get D3D Caps for device #%d. Code: %x\n", t,
					result);
				continue;
			}
			if (info->caps.DeviceType == D3DDEVTYPE_HAL)
			{
				info->type = D3DDEVTYPE_HAL;
				std::memcpy(info->description, "HAL", 4);
			}
			else if (info->caps.DeviceType == D3DDEVTYPE_REF)
			{
				info->type = D3DDEVTYPE_REF;
				std::memcpy(info->description, "REF", 4);
			}
			else
			{
				std::memcpy(info->description, "???", 4);
				JE::LogDebugString("Gfx_D3D::EnumDevices - Unknown type for device: #%d.\n", t);
			}
			info->windowed = (info->caps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) != 0;
			info->vertexShader = ((info->caps.VertexShaderVersion >> 8) & 255) != 0;
			info->pixelShader = ((info->caps.PixelShaderVersion >> 8) & 255) != 0;
			info->guardBand = info->caps.GuardBandLeft != 0 && info->caps.GuardBandRight != 0 &&
							  info->caps.GuardBandTop != 0 && info->caps.GuardBandBottom != 0;
			for (DWORD f = 0; f < formats.size(); ++f)
			{
				Gfx_D3DFormatInfo &format = formats[f];
				if (SUCCEEDED(direct3D->CheckDeviceType(a, info->type, format.display,
														format.display, FALSE)))
				{
					if (info->caps.DevCaps & D3DDEVCAPS_PUREDEVICE)
					{
						format.behavior = 80;
						format.accepted = CheckDeviceRequirements(info, direct3D, a, format);
					}
					if (!format.accepted)
					{
						format.behavior = 64;
						format.accepted = CheckDeviceRequirements(info, direct3D, a, format);
					}
					if (!format.accepted)
					{
						format.behavior = 128;
						format.accepted = CheckDeviceRequirements(info, direct3D, a, format);
					}
					if (!format.accepted)
					{
						format.behavior = 32;
						format.accepted = CheckDeviceRequirements(info, direct3D, a, format);
					}
					if (format.accepted)
					{
						format.accepted = ChooseDepthStencil(format.depth, a, info->type,
															 format.display, format.display, 16, 8);
						if (format.accepted && adapter->windowDepthStencil == D3DFMT_UNKNOWN &&
							format.display == adapter->desktop.Format)
							adapter->windowDepthStencil = format.depth;
					}
				}
			}
			std::vector<DWORD> acceptedModes, acceptedFormats;
			acceptedModes.reserve(kept);
			acceptedFormats.reserve(kept);
			for (DWORD m = 0; m < kept; ++m)
			{
				for (DWORD f = 0; f < formats.size(); ++f)
					if (formats[f].accepted && formats[f].display == modes[m].Format)
					{
						acceptedModes.push_back(m);
						acceptedFormats.push_back(f);
						break;
					}
			}
			if (!acceptedModes.empty())
			{
				info->modes = new Gfx_D3DMode[acceptedModes.size()];
				if (info->modes)
				{
					info->modeCount = acceptedModes.size();
					for (DWORD m = 0; m < acceptedModes.size(); ++m)
					{
						// Deliberately first N sorted modes, ignoring acceptedModes[m].
						Gfx_D3DMode &output = info->modes[m];
						Gfx_D3DFormatInfo &format = formats[acceptedFormats[m]];
						output.display.Width = modes[m].Width;
						output.display.Height = modes[m].Height;
						output.display.RefreshRate = modes[m].RefreshRate;
						output.display.Format = output.backBufferFormat = modes[m].Format;
						output.display.Format = output.backBufferFormat = format.backBuffer;
						output.depthStencilFormat = format.depth;
						output.stencil = format.depth == D3DFMT_D24S8 ||
										 format.depth == D3DFMT_D24X4S4 ||
										 format.depth == D3DFMT_D15S1;
						output.behavior = format.behavior;
					}
				}
				++devices;
				++adapter->deviceCount;
			}
			else if (info->modes)
			{
				delete[] info->modes;
				info->modes = 0;
			} // Incoming pointer was never initialized here.
		}
		if (devices)
		{
			adapter->deviceCount = devices;
			Gfx_D3DDeviceInfo &info = adapter->devices[0];
			for (DWORD m = 0; m < info.modeCount; ++m)
				if (info.modes[m].backBufferFormat == adapter->desktop.Format)
				{
					adapter->windowBehavior = info.modes[m].behavior;
					break;
				}
			++compatible;
		}
		else
		{
			for (DWORD d = 0; d < adapter->deviceCount; ++d)
				if (adapter->devices[d].modes)
				{
					delete[] adapter->devices[d].modes;
					adapter->devices[d].modes = 0;
				}
			if (adapter->devices)
			{
				delete[] adapter->devices;
				adapter->devices = 0;
			}
		}
		if (modes)
			delete[] modes;
		std::vector<Gfx_D3DFormatInfo>().swap(formats);
	}
	if (!compatible)
	{
		if (adapters)
		{
			delete[] adapters;
			adapters = 0;
		}
		JE::FatalError("No Compatible D3D Adapters found!\n");
	}
	adapterCount = compatible;
}
