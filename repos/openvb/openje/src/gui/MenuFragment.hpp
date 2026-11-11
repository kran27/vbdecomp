#pragma once

#include "graphics/RenderQueue.hpp"
#include <windows.h>

namespace JE
{
	namespace GUI
	{
		struct FragmentTextureInfo
		{
			LONG width, height;
		};

		// Calls belong to the actual GUI owner/texture manager. A preview may bind
		// these boundaries explicitly; it does not thereby construct the full manager.
		class FragmentServices
		{
		public:
			virtual void Scale(float& x, float& y) = 0;
			virtual const FragmentTextureInfo* TextureInfo(IDirect3DBaseTexture8* texture) = 0;
			virtual DWORD TimeMS() = 0;
			virtual bool BatchingEnabled() = 0;
			virtual void Submit(Gfx::UIMesh& mesh, Gfx::Viewport* target,
				BYTE batchClass, bool batch) = 0;
			virtual ~FragmentServices() {}
		};

		// Native counterpart of the recovered fragment fields. No legacy STL/object
		// storage is overlaid. Unknown X/Y stay unavailable until UpdateGeometry.
		class MenuFragment
		{
		public:
			MenuFragment();
			void UpdateGeometry(FragmentServices& services); // 0x4B1830.
			void UpdateUV(FragmentServices& services); // 0x4B1B00.
			void Draw(FragmentServices& services, Gfx::Viewport* target, BYTE flags); // 0x4B1720.

			RECT rectangle;
			BYTE geometryDirty, uvDirty;
			DWORD tileMode, frameWidth, frameHeight;
			DWORD frameRate, currentFrame;
			DWORD frameCount, lastFrameTime; // Unwritten by the original constructor.
			float sourceX, sourceY, sourceWidth, sourceHeight;
			DWORD color;
			Gfx::UIMesh mesh;
			bool geometryAvailable;
		};
	}
}
