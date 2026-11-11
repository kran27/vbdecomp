#include "D3DContext.hpp"
#include "D3DServices.hpp"
#include "D3DModeOrder.hpp"
#include "../diagnostics/DebugOutput.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace JE { namespace Gfx
{
	namespace
	{
		const DWORD RenderStateIds[76] = {
			7,8,9,10,14,15,16,19,20,22,23,24,25,26,27,28,29,30,34,35,36,37,38,40,
			47,48,52,53,54,55,56,57,58,59,60,128,129,130,131,132,133,134,135,136,
			137,139,140,141,142,143,145,146,147,148,151,152,153,154,155,156,
			157,158,159,160,161,162,163,164,165,166,167,168,170,171,172,173
		};
		const DWORD TextureStateIds[27] = {
			1,2,3,4,5,6,7,8,9,10,11,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28
		};
		// Original global dirty lists are reset by the baseline initialization APIs.
		WORD renderDirty[87];
		BYTE textureDirty[232];
		DWORD textureDirtyCount, textureDirtyCursor;

		bool HasStencil(D3DFORMAT format)
		{ return format == D3DFMT_D24S8 || format == D3DFMT_D24X4S4 || format == D3DFMT_D15S1; }

		DWORD FloatBits(float value)
		{
			DWORD bits;
			std::memcpy(&bits, &value, sizeof(bits));
			return bits;
		}

		struct FormatCandidate
		{
			FormatCandidate()
				: behavior(0), accepted(0), acceptanceKnown(false), depth(D3DFMT_UNKNOWN) {}
			D3DFORMAT display, backBuffer;
			DWORD behavior; // Backing value is not consumed before acceptance is known.
			BYTE accepted;
			bool acceptanceKnown;
			D3DFORMAT depth;
		};

		struct DisplayFormatEqual
		{
			explicit DisplayFormatEqual(D3DFORMAT value) : value(value) {}
			bool operator()(const FormatCandidate& format) const { return format.display == value; }
			D3DFORMAT value;
		};

		bool Acceptance(const FormatCandidate& format)
		{
			if (!format.acceptanceKnown)
				throw std::logic_error("Gfx_D3D 0x5047A0 consumed an unwritten format-acceptance byte");
			return format.accepted != 0;
		}
	}

	bool DisplayModeLess(const D3DDISPLAYMODE& left, const D3DDISPLAYMODE& right)
	{
		return CompareD3DDisplayModes(&left, &right) < 0;
	}

	void D3DContext::Initialize(BYTE allowReference)
	{
		std::memset(&identity, 0, sizeof(identity));
		identity._11 = identity._22 = identity._33 = identity._44 = 1.0f;
		windowMode = DeviceMode();
		device = NULL;
		std::memset(&presentation, 0, sizeof(presentation));
		matrixStack = NULL;
		ready = 0;
		textureStageCount = 0;
		textureStates.clear();
		adapterCount = 0;
		adapters.clear();
		currentAdapter = currentDeviceIndex = currentModeIndex = 0;
		currentDevice = NULL;
		currentMode = NULL;
		requestedWindowed = 0;
		D3DServices::PerformanceFrequency(&performanceFrequency);
		field0x6B4 = 0;
		std::memset(statistics, 0, sizeof(statistics));
		verticalSync = 1;
		inverseFrequency = static_cast<float>(1.0 / static_cast<double>(performanceFrequency.QuadPart));
		direct3D = D3DServices::Create(D3D_SDK_VERSION);
		if (!direct3D)
			JE::FatalError("Failed to Create D3D!  Please ensure that DX8.1 is installed properly.\n");
		JE::LogDebugString("Created D3D\n");
		EnumerateDevices(allowReference);
	}

	bool D3DContext::CheckDeviceRequirements(const DeviceInfo& info, UINT adapter,
		D3DFORMAT backBufferFormat, DWORD behavior)
	{
		if (info.type != D3DDEVTYPE_HAL) return true;
		bool accepted = true;
		if ((behavior & D3DCREATE_HARDWARE_VERTEXPROCESSING) && info.caps.MaxVertexBlendMatrices < 4)
		{ accepted = false; JE::LogDebugString("[Enum] HWVP requires 4 blend matrices!\n"); }
		if (info.caps.MaxVertexIndex < 65535)
		{ accepted = false; JE::LogDebugString("[Enum] Device doesn't support minimum of 65536 indices!\n"); }
		if (info.caps.MaxPrimitiveCount < 65535)
		{ accepted = false; JE::LogDebugString("[Enum] Device doesn't support minimum of 65536 primitives !\n"); }
		if (info.caps.MaxSimultaneousTextures < 2)
		{ accepted = false; JE::LogDebugString("[Enum] Device doesn't support minimum of 2 simultaneous textures!\n"); }
		if (info.caps.MaxTextureWidth < 1024 || info.caps.MaxTextureHeight < 1024)
		{ accepted = false; JE::LogDebugString("[Enum] Device doesn't support minimum texture size of 1024x1024!\n"); }
		if (!(info.caps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN))
		{ accepted = false; JE::LogDebugString("[Enum] Device doesn't support TEXGEN!\n"); }
		if (!(info.caps.TextureOpCaps & 0x105F))
		{ accepted = false; JE::LogDebugString("[Enum] Device doesn't support required TextureOp!\n"); }
		const D3DFORMAT formats[] =
			{D3DFMT_DXT1, D3DFMT_DXT2, D3DFMT_DXT3, D3DFMT_DXT4, D3DFMT_DXT5};
		for (UINT index = 0; index < sizeof(formats) / sizeof(formats[0]); ++index)
		{
			const D3DFORMAT format = formats[index];
			if (FAILED(D3DServices::CheckDeviceFormat(direct3D, adapter, info.type, backBufferFormat,
				0, D3DRTYPE_TEXTURE, format)))
			{
				JE::LogDebugString("[Enum] Device doesn't support DXT texture format: %x\n", format);
				return false;
			}
		}
		return accepted;
	}

	bool D3DContext::ChooseDepthStencil(D3DFORMAT& output, UINT adapter, D3DDEVTYPE type,
		D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat)
	{
		const D3DFORMAT depths[] = {D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D15S1,
			D3DFMT_D32, D3DFMT_D24X8, D3DFMT_D16};
		for (UINT index = 0; index < sizeof(depths) / sizeof(depths[0]); ++index)
		{
			const D3DFORMAT depth = depths[index];
			if (SUCCEEDED(D3DServices::CheckDeviceFormat(direct3D, adapter, type, adapterFormat,
					D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, depth)) &&
				SUCCEEDED(D3DServices::CheckDepthStencilMatch(direct3D, adapter, type,
					adapterFormat, backBufferFormat, depth)))
			{
				output = depth;
				return true;
			}
		}
		output = D3DFMT_UNKNOWN;
		return false;
	}

	void D3DContext::EnumerateDevices(BYTE allowReference)
	{
		const UINT count = D3DServices::AdapterCount(direct3D);
		if (!count)
			JE::FatalError("No D3D Adapters found!  Please ensure that DX8.1 device drivers is installed properly.\n");
		adapters.resize(count);
		UINT compatible = 0;
		for (UINT adapter = 0; adapter < count; ++adapter)
		{
			AdapterInfo& output = adapters[compatible];
			D3DServices::AdapterDisplayMode(direct3D, adapter, &output.desktop);
			output.windowDepthStencil = D3DFMT_UNKNOWN;
			const HRESULT identifierResult = D3DServices::AdapterIdentifier(direct3D, adapter, &output.identifier);
			if (FAILED(identifierResult))
			{
				JE::LogDebugString("Gfx_D3D::EnumDevices - GetAdapterIdentifier() failed: Code: %x\n", identifierResult);
				continue;
			}
			const UINT modeCount = D3DServices::AdapterModeCount(direct3D, adapter);
			std::vector<D3DDISPLAYMODE> modes;
			std::vector<FormatCandidate> formats;
			modes.reserve(modeCount);
			formats.reserve(5);
			for (UINT mode = 0; mode < modeCount; ++mode)
			{
				D3DDISPLAYMODE display;
				const HRESULT result = D3DServices::AdapterMode(direct3D, adapter, mode, &display);
				if (FAILED(result))
					JE::LogDebugString("Gfx_D3D::EnumDevices - EnumAdapterModes() failed! Code: %x\n", result);
				else if (display.Width >= 640 && display.Height >= 480)
				{
					modes.push_back(display);
					if (std::find_if(formats.begin(), formats.end(),
						DisplayFormatEqual(display.Format)) == formats.end())
					{
						FormatCandidate format;
						format.display = format.backBuffer = display.Format;
						formats.push_back(format);
					}
				}
			}
			std::sort(modes.begin(), modes.end(), DisplayModeLess);
			output.devices.clear();
			output.devices.reserve(allowReference ? 2 : 1);
			const D3DDEVTYPE types[] = {D3DDEVTYPE_HAL, D3DDEVTYPE_REF};
			for (UINT typeIndex = 0; typeIndex < (allowReference ? 2u : 1u); ++typeIndex)
			{
				DeviceInfo info;
				const HRESULT result = D3DServices::DeviceCaps(direct3D, adapter, types[typeIndex], &info.caps);
				if (FAILED(result))
				{
					JE::LogDebugString("Gfx_D3D::EnumDevices - Failed to get D3D Caps for device #%d. Code: %x\n", typeIndex, result);
					continue;
				}
				if (info.caps.DeviceType == D3DDEVTYPE_HAL)
				{ info.type = D3DDEVTYPE_HAL; std::memcpy(info.description, "HAL", sizeof(info.description)); }
				else if (info.caps.DeviceType == D3DDEVTYPE_REF)
				{ info.type = D3DDEVTYPE_REF; std::memcpy(info.description, "REF", sizeof(info.description)); }
				else
					throw std::logic_error("Gfx_D3D 0x5047A0 SDK returned an unwritten device-type domain");
				info.windowedSupported = (info.caps.Caps2 & D3DCAPS2_CANRENDERWINDOWED) != 0;
				info.vertexShaderSupported = ((info.caps.VertexShaderVersion >> 8) & 255) != 0;
				info.pixelShaderSupported = ((info.caps.PixelShaderVersion >> 8) & 255) != 0;
				info.guardBandSupported = info.caps.GuardBandLeft != 0 && info.caps.GuardBandRight != 0 &&
					info.caps.GuardBandTop != 0 && info.caps.GuardBandBottom != 0;
				for (std::vector<FormatCandidate>::iterator it = formats.begin(); it != formats.end(); ++it)
				{
					FormatCandidate& format = *it;
					if (SUCCEEDED(D3DServices::CheckDeviceType(direct3D, adapter, info.type,
						format.display, format.display, FALSE)))
					{
						if (info.caps.DevCaps & D3DDEVCAPS_PUREDEVICE)
						{
							format.behavior = D3DCREATE_PUREDEVICE | D3DCREATE_HARDWARE_VERTEXPROCESSING;
							format.accepted = CheckDeviceRequirements(info, adapter, format.backBuffer, format.behavior);
							format.acceptanceKnown = true;
						}
						if (!Acceptance(format))
						{
							const DWORD behaviors[] = {D3DCREATE_HARDWARE_VERTEXPROCESSING,
								D3DCREATE_MIXED_VERTEXPROCESSING, D3DCREATE_SOFTWARE_VERTEXPROCESSING};
							for (UINT index = 0; index < sizeof(behaviors) / sizeof(behaviors[0]); ++index)
							{
								const DWORD behavior = behaviors[index];
								format.behavior = behavior;
								format.accepted = CheckDeviceRequirements(info, adapter, format.backBuffer, behavior);
								format.acceptanceKnown = true;
								if (format.accepted) break;
							}
						}
						if (Acceptance(format))
						{
							format.accepted = ChooseDepthStencil(format.depth, adapter, info.type, format.display, format.display);
							if (format.accepted && output.windowDepthStencil == D3DFMT_UNKNOWN && format.display == output.desktop.Format)
								output.windowDepthStencil = format.depth;
						}
					}
				}
				std::vector<UINT> acceptedFormats;
				for (std::vector<D3DDISPLAYMODE>::const_iterator it = modes.begin(); it != modes.end(); ++it)
				{
					const D3DDISPLAYMODE& display = *it;
					for (UINT format = 0; format < formats.size(); ++format)
					{
						if (Acceptance(formats[format]) && formats[format].display == display.Format)
						{ acceptedFormats.push_back(format); break; }
					}
				}
				for (UINT index = 0; index < acceptedFormats.size(); ++index)
				{
					// Original publishes the first N sorted modes, ignoring its accepted-mode indices.
					const D3DDISPLAYMODE& display = modes[index];
					const FormatCandidate& format = formats[acceptedFormats[index]];
					DeviceMode mode;
					mode.width = display.Width; mode.height = display.Height; mode.refreshRate = display.RefreshRate;
					mode.displayFormat = mode.backBufferFormat = format.backBuffer;
					mode.depthStencilFormat = format.depth; mode.behavior = format.behavior;
					mode.stencil = HasStencil(format.depth);
					info.modes.push_back(mode);
				}
				if (!info.modes.empty()) output.devices.push_back(info);
			}
			if (!output.devices.empty())
			{
				const std::vector<DeviceMode>& deviceModes = output.devices.front().modes;
				for (std::vector<DeviceMode>::const_iterator it = deviceModes.begin(); it != deviceModes.end(); ++it)
				{
					const DeviceMode& mode = *it;
					if (mode.backBufferFormat == output.desktop.Format)
					{ output.windowBehavior = mode.behavior; output.windowBehaviorKnown = true; break; }
				}
				++compatible;
			}
		}
		if (!compatible)
		{
			adapters.clear();
			JE::FatalError("No Compatible D3D Adapters found!\n");
		}
		adapterCount = compatible;
	}

	bool D3DContext::FindMode(UINT adapter, UINT deviceIndex, UINT width, UINT height,
		UINT refreshRate, BYTE color32, UINT& output) const
	{
		if (adapter >= adapterCount || deviceIndex >= adapters[adapter].devices.size()) return false;
		const std::vector<DeviceMode>& modes = adapters[adapter].devices[deviceIndex].modes;
		for (output = 0; output < modes.size(); ++output)
		{
			const DeviceMode& mode = modes[output];
			if (mode.width != width || mode.height != height) continue;
			const int format = mode.backBufferFormat;
			if (color32 ? (format != 21 && format != 22) :
				(format != 23 && format != 24 && format != 25 && format != 26 && format != 30)) continue;
			if (mode.refreshRate >= (refreshRate ? refreshRate : 60)) return true;
		}
		return false;
	}

	HRESULT D3DContext::CreateDevice(HWND owner, UINT adapter, UINT deviceIndex, UINT mode,
		BYTE windowed, BYTE multithreaded)
	{
		if (!direct3D) return static_cast<HRESULT>(0x8876086C);
		AdapterInfo& selected = adapters[adapter];
		currentAdapter = adapter; currentDeviceIndex = deviceIndex;
		window = owner;
		currentDevice = &selected.devices[deviceIndex];
		currentModeIndex = mode;
		currentMode = &currentDevice->modes[mode];
		requestedWindowed = windowed;
		if (windowed && currentDevice->windowedSupported)
		{
			RECT rect;
			D3DServices::ClientRect(owner, &rect);
			presentation.BackBufferWidth = rect.right;
			presentation.BackBufferHeight = rect.bottom;
			presentation.BackBufferFormat = selected.desktop.Format;
			presentation.Windowed = TRUE;
			presentation.FullScreen_RefreshRateInHz = presentation.FullScreen_PresentationInterval = 0;
			presentation.AutoDepthStencilFormat = selected.windowDepthStencil;
			currentMode = &windowMode;
			if (!selected.windowBehaviorKnown)
				throw std::logic_error("Gfx_D3D 0x503920 consumed an unwritten window behavior DWORD");
			windowMode.behavior = selected.windowBehavior;
			windowMode.width = rect.right; windowMode.height = rect.bottom; windowMode.refreshRate = 0;
			windowMode.displayFormat = windowMode.backBufferFormat = selected.desktop.Format;
			windowMode.depthStencilFormat = selected.windowDepthStencil;
			windowMode.stencil = HasStencil(selected.windowDepthStencil);
		}
		else
		{
			presentation.BackBufferWidth = currentMode->width;
			presentation.BackBufferHeight = currentMode->height;
			presentation.BackBufferFormat = currentMode->backBufferFormat;
			presentation.Windowed = FALSE;
			presentation.FullScreen_RefreshRateInHz = currentMode->refreshRate;
			presentation.FullScreen_PresentationInterval = verticalSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
			presentation.AutoDepthStencilFormat = currentMode->depthStencilFormat;
			const int height = D3DServices::SystemMetrics(SM_CYSCREEN);
			const int width = D3DServices::SystemMetrics(SM_CXSCREEN);
			D3DServices::Move(owner, 0, 0, width, height, TRUE);
		}
		presentation.BackBufferCount = 1;
		presentation.SwapEffect = D3DSWAPEFFECT_DISCARD;
		presentation.EnableAutoDepthStencil = TRUE;
		presentation.MultiSampleType = D3DMULTISAMPLE_NONE;
		presentation.hDeviceWindow = owner;
		presentation.Flags = 0;
		const HRESULT result = D3DServices::CreateDevice(direct3D, currentAdapter, currentDevice->type, owner,
			currentMode->behavior | (multithreaded ? D3DCREATE_MULTITHREADED : 0), &presentation, &device);
		if (FAILED(result)) JE::FatalError("Failed to create D3DDevice (%x)\n", result);
		JE::LogDebugString("Created D3DDevice");
		const HRESULT stackResult = MatrixStackBridge::Create(0, &matrixStack);
		if (FAILED(stackResult)) JE::LogDebugString("Gfx_D3D::CreateDevice - Failed to create matrix stack");
		statistics[14] = D3DServices::AvailableTextureMemory(device);
		JE::LogDebugString("Free Texture Memory: %d\n", statistics[14]);
		textureStageCount = currentDevice->caps.MaxTextureBlendStages;
		textureStates = std::vector<TextureStageState>(textureStageCount);
		defaultTextureStates = std::vector<TextureStageState>(textureStageCount);
		InitializeRenderStates();
		InitializeTextureStates();
		field0xA8 = 0xFFFF; field0xAC = 101; field0xA4 = 2;
		ApplyStates(1);
		ready = 1;
		return stackResult;
	}

	void D3DContext::InitializeRenderStates()
	{
		std::memset(defaultRenderStates, 0, sizeof(defaultRenderStates));
		std::memset(renderDirty, 0, sizeof(renderDirty));
		defaultRenderStates[36] = 0u;
		defaultRenderStates[25] = 8u;
		defaultRenderStates[56] = 8u;
		defaultRenderStates[37] = 1065353216u;
		defaultRenderStates[38] = 1065353216u;
		defaultRenderStates[7] = 1u;
		defaultRenderStates[8] = 3u;
		defaultRenderStates[9] = 2u;
		defaultRenderStates[10] = 0u;
		defaultRenderStates[14] = 1u;
		defaultRenderStates[15] = 0u;
		defaultRenderStates[16] = 1u;
		defaultRenderStates[19] = 2u;
		defaultRenderStates[20] = 1u;
		defaultRenderStates[22] = 3u;
		defaultRenderStates[23] = 4u;
		defaultRenderStates[24] = 0u;
		defaultRenderStates[26] = 0u;
		defaultRenderStates[27] = 0u;
		defaultRenderStates[28] = 0u;
		defaultRenderStates[29] = 0u;
		defaultRenderStates[30] = 0u;
		defaultRenderStates[34] = 0u;
		defaultRenderStates[35] = 0u;
		defaultRenderStates[40] = 0u;
		defaultRenderStates[47] = 0u;
		defaultRenderStates[48] = 0u;
		defaultRenderStates[52] = 0u;
		defaultRenderStates[53] = 1u;
		defaultRenderStates[54] = 1u;
		defaultRenderStates[55] = 1u;
		defaultRenderStates[57] = 0u;
		defaultRenderStates[58] = 0xFFFFFFFFu;
		defaultRenderStates[59] = 0xFFFFFFFFu;
		defaultRenderStates[60] = 0u;
		defaultRenderStates[128] = 0u;
		defaultRenderStates[129] = 0u;
		defaultRenderStates[130] = 0u;
		defaultRenderStates[131] = 0u;
		defaultRenderStates[132] = 0u;
		defaultRenderStates[133] = 0u;
		defaultRenderStates[134] = 0u;
		defaultRenderStates[135] = 0u;
		defaultRenderStates[136] = 1u;
		defaultRenderStates[137] = 1u;
		defaultRenderStates[139] = 0u;
		defaultRenderStates[140] = 0u;
		defaultRenderStates[141] = 1u;
		defaultRenderStates[142] = 1u;
		defaultRenderStates[143] = 0u;
		defaultRenderStates[145] = 1u;
		defaultRenderStates[146] = 2u;
		defaultRenderStates[147] = 2u;
		defaultRenderStates[148] = 0u;
		defaultRenderStates[151] = 0u;
		defaultRenderStates[152] = 0u;
		defaultRenderStates[154] = 1065353216u;
		defaultRenderStates[155] = 1065353216u;
		defaultRenderStates[158] = 1065353216u;
		defaultRenderStates[159] = 0u;
		defaultRenderStates[160] = 0u;
		defaultRenderStates[156] = 0u;
		defaultRenderStates[157] = 0u;
		defaultRenderStates[162] = 0xFFFFFFFFu;
		defaultRenderStates[163] = 0u;
		defaultRenderStates[164] = 1065353216u;
		defaultRenderStates[165] = 0u;
		defaultRenderStates[161] = 1u;
		defaultRenderStates[167] = 0u;
		defaultRenderStates[168] = 15u;
		defaultRenderStates[170] = 0u;
		defaultRenderStates[171] = 1u;
		defaultRenderStates[172] = 3u;
		defaultRenderStates[173] = 1u;
		defaultRenderStates[153] = currentDevice->type == D3DDEVTYPE_SW ||
			(currentDevice->caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0;
		defaultRenderStates[166] = FloatBits(currentDevice->caps.MaxPointSize);
	}

	UINT D3DContext::InitializeTextureStates()
	{
		textureDirtyCount = textureDirtyCursor = 0;
		std::memset(textureDirty, 0, sizeof(textureDirty));
		UINT stage = 0;
		for (; stage < textureStageCount; ++stage)
		{
			TextureStageState& entry = defaultTextureStates[stage];
			entry.texture = NULL;
			std::memset(entry.values, 0, sizeof(entry.values));
			DWORD* values = entry.values;
			// One unused DWORD follows the texture; state N is values[N].
			values[1] = stage ? 1 : 4; values[2] = 2; values[3] = 1;
			values[4] = stage ? 1 : 2; values[5] = 2; values[6] = 1;
			values[11] = stage;
			const UINT enabledStates[] = {13,14,16,17,21,25};
			for (UINT index = 0; index < sizeof(enabledStates) / sizeof(enabledStates[0]); ++index)
				values[enabledStates[index]] = 1;
			if ((currentDevice->caps.TextureOpCaps >> 24) & 3)
				values[26] = values[27] = 0;
			if (currentDevice->caps.PrimitiveMiscCaps & 0x400) values[28] = 1;
		}
		return stage;
	}

	HRESULT D3DContext::ApplyDefaultRenderStates()
	{
		std::memcpy(renderStates, defaultRenderStates, sizeof(renderStates));
		HRESULT result = 0;
		for (UINT index = 0; index < sizeof(RenderStateIds) / sizeof(RenderStateIds[0]); ++index)
		{
			const DWORD state = RenderStateIds[index];
			result = D3DServices::RenderState(device, static_cast<D3DRENDERSTATETYPE>(state), renderStates[state]);
		}
		return result;
	}

	UINT D3DContext::ApplyDefaultTextureStates()
	{
		textureStates = defaultTextureStates;
		UINT result = 0;
		for (UINT stage = 0; stage < textureStageCount; ++stage)
		{
			for (UINT index = 0; index < sizeof(TextureStateIds) / sizeof(TextureStateIds[0]); ++index)
			{
				const DWORD state = TextureStateIds[index];
				D3DServices::TextureStageState(device, stage, static_cast<D3DTEXTURESTAGESTATETYPE>(state),
					textureStates[stage].values[state]);
			}
			result = textureStageCount;
		}
		return result;
	}

	DWORD D3DContext::ApplyStates(BYTE resetDefaults)
	{
		if (resetDefaults)
		{
			ApplyDefaultRenderStates();
			return ApplyDefaultTextureStates();
		}
		DWORD result = 0;
		for (UINT index = 0; index < sizeof(RenderStateIds) / sizeof(RenderStateIds[0]); ++index)
		{
			const DWORD state = RenderStateIds[index];
			result = D3DServices::RenderState(device, static_cast<D3DRENDERSTATETYPE>(state), renderStates[state]);
		}
		for (UINT stage = 0; stage < textureStageCount; ++stage)
		{
			for (UINT index = 0; index < sizeof(TextureStateIds) / sizeof(TextureStateIds[0]); ++index)
			{
				const DWORD state = TextureStateIds[index];
				D3DServices::TextureStageState(device, stage, static_cast<D3DTEXTURESTAGESTATETYPE>(state),
					textureStates[stage].values[state]);
			}
			result = textureStageCount;
		}
		return result;
	}

	HRESULT D3DContext::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
	{
		HRESULT result = 0;
		if (renderStates[state] != value)
		{
			result = D3DServices::RenderState(device, state, value);
			renderStates[state] = value;
		}
		return result;
	}

	HRESULT D3DContext::SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix)
	{
		const HRESULT result = D3DServices::Transform(device, state, matrix);
		++statistics[10];
		return result;
	}

	HRESULT D3DContext::SetTexture(UINT stage, IDirect3DBaseTexture8* texture)
	{
		if (stage >= textureStageCount)
		{
			JE::LogDebugString("DeviceSetTexture failed - Stage: %d < TextureStageCount: %d\n", stage, textureStageCount);
			return static_cast<HRESULT>(0x8876086A);
		}
		HRESULT result = 0;
		if (textureStates[stage].texture != texture)
		{
			result = D3DServices::Texture(device, stage, texture);
			textureStates[stage].texture = texture;
			++statistics[8];
		}
		return result;
	}

	HRESULT D3DContext::SetTextureStageState(UINT stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
	{
		HRESULT result = 0;
		if (textureStates[stage].values[state] != value)
		{
			result = D3DServices::TextureStageState(device, stage, state, value);
			textureStates[stage].values[state] = value;
		}
		return result;
	}

	void D3DContext::ReleaseDevice()
	{
		if (matrixStack)
		{
			MatrixStackBridge::Release(matrixStack);
			matrixStack = NULL;
		}
		if (device)
			for (UINT stage = 0; stage < textureStageCount; ++stage)
			{
				const HRESULT result = SetTexture(stage, NULL);
				if (FAILED(result)) JE::LogDebugString("DeviceSetTexture(NULL) failed! (%x)\n", result);
			}
		std::vector<TextureStageState>().swap(defaultTextureStates);
		std::vector<TextureStageState>().swap(textureStates);
		if (device)
		{
			D3DServices::Release(device);
			device = NULL;
		}
	}

	void D3DContext::Shutdown()
	{
		ReleaseDevice();
		if (direct3D)
		{
			D3DServices::Release(direct3D);
			direct3D = NULL;
		}
		std::vector<AdapterInfo>().swap(adapters);
		JE::LogDebugString("Destroyed D3D\n");
	}
}}
