#include "Manager.hpp"

namespace GUI
{
	TextureMask* Manager::GetTextureMask(DWORD handle)
	{
		return textureMasks.Get(handle);
	}
}
