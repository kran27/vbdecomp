#include "Fragment.hpp"
#include "Manager.hpp"
#include "PacketManager.hpp"
#include "graphics/SystemInterface.hpp"
#include <mmsystem.h>

namespace GUI
{
	void Fragment::Draw(Gfx_Viewport* target, BYTE flags)
	{
		if (frameRate)
		{
			DWORD now = timeGetTime();
			DWORD elapsed = now - lastFrameTime;
			DWORD interval = 1000 / static_cast<LONG>(frameRate);
			if (elapsed > interval)
			{
				DWORD steps = elapsed / interval;
				DWORD previousFrame = currentFrame;
				lastFrameTime = now;
				uvDirty = 1;
				currentFrame = static_cast<LONG>(steps + previousFrame)
					% static_cast<LONG>(frameCount);
			}
		}
		if (uvDirty) UpdateUV();
		CPktManager* packets = g_Manager.packets;
		if (!packets->optimizationLevel)
		{
			g_GfxSystem->QueueMeshInstances(&instance, target, 1);
			return;
		}
		BYTE alpha = static_cast<BYTE>(GetColor() >> 24);
		if (alpha == 255)
		{
			if (flags & 1)
			{
				if (flags & 2) packets->Submit(&instance, target, 2);
				else packets->Submit(&instance, target, 1);
			}
			else packets->Submit(&instance, target, 0);
		}
		else if (alpha > 0)
		{
			if (flags & 2) packets->Submit(&instance, target, 2);
			else packets->Submit(&instance, target, 1);
		}
	}
}
