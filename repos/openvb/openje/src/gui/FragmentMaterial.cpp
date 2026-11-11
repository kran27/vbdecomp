#include "Fragment.hpp"

namespace GUI
{
	void Fragment::SetTextureHandle(DWORD handle)
	{
		material->textureHandles[0] = handle;
		uvDirty = 1;
	}

	DWORD Fragment::GetTextureHandle() { return material->textureHandles[0]; }

	void Fragment::SetColor(DWORD value)
	{
		mesh.materialSet.SetColors(1, 1, value, 0);
		if ((color & 0xFF000000) == 0xFF000000)
		{
			if ((value & 0xFF000000) != 0xFF000000)
			{
				mesh.materialSet.RebuildGroups();
				color = value;
				return;
			}
		}
		else if ((value & 0xFF000000) == 0xFF000000)
		{
			mesh.materialSet.RebuildGroups();
		}
		color = value;
	}

	void Fragment::SetMaterialModes(DWORD value)
	{
		Gfx_Material* selected = material;
		selected->mode28 = value;
		selected->mode24 = value;
		mesh.materialSet.RebuildGroups();
	}
}
