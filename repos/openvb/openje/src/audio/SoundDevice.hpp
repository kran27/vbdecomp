#pragma once

#include <windows.h>
#include <dsound.h>
#include "network/Types.hpp"
#include <cstddef>

namespace JE
{
	namespace SoundDeviceServices
	{
		HRESULT Create(LPCGUID device, IDirectSound8** output, IUnknown* outer);
	}
}
