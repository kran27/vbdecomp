#include "D3DState.hpp"
#include "../diagnostics/DebugOutput.hpp"

HRESULT Gfx_D3D::ResetDevice(const D3DPRESENT_PARAMETERS* parameters)
{
	char error[256];
	if (parameters)
	{
		presentation = *parameters;
	}
	HRESULT result = D3DERR_INVALIDCALL;
	if (device)
	{
		result = device->Reset(&presentation);
		if (SUCCEEDED(result))
		{
			ApplyCachedStates(0);
		}
		else
		{
			JE::LogDebugString("DeviceReset failed!\n");
			D3DXGetErrorStringA(result, error, sizeof(error));
			JE::LogDebugString("DXErr: %s\n", error);
		}
	}
	else
	{
		JE::LogDebugString("DeviceReset: Device not created yet\n");
	}
	return result;
}
