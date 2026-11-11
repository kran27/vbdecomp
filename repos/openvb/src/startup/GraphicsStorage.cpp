//Canonical Graphics storage. The separate scene retains its own globals and
//explicit SDK lifetime; this translation unit belongs to the native game.
#include "graphics/D3DState.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/SkinManager.hpp"
#include "gui/Manager.hpp"
#include <cstdlib>

Gfx_D3D g_D3D;
Gfx_System_Interface* g_GfxSystem=NULL;
Gfx_SkinMeshManager* g_SkinMeshManager=NULL;
namespace GUI { Manager g_Manager; } //Canonical static GUI owner; full Client startup is separate.

namespace
{
	void DestroyD3DGlobal() //Compiler cleanup645E70 forwards to proprietary503F80.
	{
		g_D3D.Shutdown();
	}

	struct D3DStaticLifetime
	{
		D3DStaticLifetime()
		{
			g_D3D.Initialize(0); //6419B0: actual pre-main505170, including enumeration.
			std::atexit(&DestroyD3DGlobal); //Register only after initialization returns.
		}
	};
	D3DStaticLifetime d3dStaticLifetime;
}

//Actual separate reset-only storage, cleared by502CB0/502F40. Its element
//meaning remains unknown; these definitions add no initialization policy.
BYTE g_D3DRenderResetBytes[174];
BYTE g_D3DTextureResetBytes[232];
DWORD g_D3DTextureResetValue0, g_D3DTextureResetValue1;
