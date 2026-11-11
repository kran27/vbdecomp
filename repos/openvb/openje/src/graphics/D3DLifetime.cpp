#include "D3DState.hpp"
#include "../diagnostics/DebugOutput.hpp"

void Gfx_D3D::ReleaseDevice()
{
	if (matrixStack) { matrixStack->Release(); matrixStack = 0; }
	if (device)
	{
		for (DWORD stage = 0; stage < textureStageCount; ++stage)
		{
			HRESULT result = SetTexture(stage, 0);
			if (FAILED(result)) JE::LogDebugString("DeviceSetTexture(NULL) failed! (%x)\n", result);
		}
	}
	if (defaultTextureStates) { delete[] defaultTextureStates; defaultTextureStates = 0; }
	if (textureStates) { delete[] textureStates; textureStates = 0; }
	if (device) { device->Release(); device = 0; }
}

void Gfx_D3D::Shutdown()
{
	ReleaseDevice();
	if (direct3D) { direct3D->Release(); direct3D = 0; }
	for (DWORD a = 0; a < adapterCount; ++a)
	{
		Gfx_D3DAdapterInfo* adapter = &adapters[a];
		for (DWORD d = 0; d < adapter->deviceCount; ++d)
		{
			Gfx_D3DDeviceInfo* info = &adapter->devices[d];
			if (info->modes) { delete[] info->modes; info->modes = 0; }
		}
		if (adapter->devices) { delete[] adapter->devices; adapter->devices = 0; }
	}
	if (adapters) { delete[] adapters; adapters = 0; }
	JE::LogDebugString("Destroyed D3D\n");
}
