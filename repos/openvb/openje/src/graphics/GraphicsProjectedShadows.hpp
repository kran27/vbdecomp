#pragma once

#include <windows.h>

class Gfx_Viewport;
namespace JE
{
	class GraphicsLevelResources;
	//Known Engine+5C/+60/+64/+EC field family, owned as normal C++ state.
	//The enable byte belongs to EngineFrameState; no second flag is stored here.
	class GraphicsProjectedShadows
	{
	public:
		GraphicsProjectedShadows(); //5BFE10: capacity50, count0, 12-byte points.
		~GraphicsProjectedShadows();
		void SetCapacity(UINT size); //5BDF80.
		void SetTexture(DWORD handle); //5BDEE0; validated borrowed handle.
		void ReleaseTexture(); //5C1C05..1D: Engine's actual shutdown subset.
		void SetEnabled(BYTE& engineFlag, Gfx_Viewport& defaultViewport, BYTE value); //5BDB20.
		void AddPoint(BYTE enabled, const float* point); //5BDFC0: x,z,radius.
		void Render(GraphicsLevelResources& level); //5BEF80.

		struct Point { float x, z, radius; };
		UINT capacity, count;
		Point* points;
		DWORD textureHandle;
	private:
		GraphicsProjectedShadows(const GraphicsProjectedShadows&);
		GraphicsProjectedShadows& operator=(const GraphicsProjectedShadows&);
	};
}
