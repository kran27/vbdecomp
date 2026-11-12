#include "Engine.hpp"
#include "TextureManager.hpp"
#include "BufferManager.hpp"
#include "ModelDataManager.hpp"
#include "SkinManager.hpp"
#include "Viewport.hpp"
#include "D3DState.hpp"
#include "BuiltinVertexShaders.hpp"
#include "RenderStyleRegistry.hpp"
#include <new>
#include <cstring>

Gfx_Engine::Gfx_Engine()
	: automapWidth(0), automapHeight(0), levelMinimum(0, 0, 0), levelMaximum(0, 0, 0),
	levelWidth(0), levelHeight(0), maskWidth(0), maskHeight(0),
	exploredGray(0xFF808080), visibleGray(0xFFFFFFFF), statisticsFont(_T("Arial"), 12, 0)
{
	// Normal list and both strings construct in their original order; the
	// initializer writes above precede the actual CD3DFont constructor. Only
	// the following original scalar writes belong to the constructor body.
	value04 = 0;
	value08 = 0;
	value0C = 0;
	value10 = 0;
	statistics = 0;
	window = NULL;
	value44 = 0;
	transparency = 0;
	lighting = 0;
	value88F = 0;
	value890 = 0;
	shadowPointCapacity = 0;
	shadowPointCount = 0;
	shadowPoints = NULL;
	fogOfWar = 0;
	revealExplored = 1;
	automapRequested = 0;
	automapReady = 1;
	automapRenderTarget = NULL;
	automapDepthSurface = NULL;
	automapTexture = 0;
	colorFormat = D3DFMT_UNKNOWN;
	depthFormat = D3DFMT_UNKNOWN;
	automapMaskHandle = 0;
	visibleMaskHandle = 0;
	fogOfWarTarget = NULL;
	shadowWidth = 0;
	shadowHeight = 0;
	projectedShadows = 0;
	projectedShadowTexture = 0;
	projectedShadowTarget = NULL;
	textureF4 = 0;
	resource894 = NULL;
	resource898 = NULL;
	resource89C = NULL;
	resource8A0 = NULL;
	currentViewport = NULL;
	defaultViewport = NULL;
	postViewport = NULL;
	textureManager = new Gfx_TextureManager;
	bufferManager = new Gfx_BufferManager;
	modelDataManager = new Gfx_ModelDataManager;
	skinManager = new Gfx_SkinMeshManager;
	lostVideoCallback = NULL;
	restoredVideoCallback = NULL;
	activeLightLimit = 0;
	Gfx_Engine::ResetParameters84And88();
	Gfx_Engine::SetProjectedShadowPointCapacity(50);
	// value34, padding and statisticsText retain their original unwritten state.
}

Gfx_Engine::~Gfx_Engine()
{
	Gfx_Engine::ShutdownDevice();
	if (modelDataManager)
	{
		delete modelDataManager;
		modelDataManager = NULL;
	}
	if (textureManager)
	{
		delete textureManager;
		textureManager = NULL;
	}
	if (bufferManager)
	{
		delete bufferManager;
		bufferManager = NULL;
	}
	if (skinManager)
	{
		delete skinManager;
		skinManager = NULL;
	}
	if (shadowPoints)
	{
		::operator delete(shadowPoints);
		shadowPoints = NULL;
	}
	// Compiler/real-library destruction follows: SDK font, mapName,
	// automapTextureName, borrowed viewport-list nodes, then base publication.
}

void Gfx_Engine::ShutdownDevice()
{
	if (resource894) { resource894->Release(); resource894 = NULL; }
	if (resource898) { resource898->Release(); resource898 = NULL; }
	if (resource89C) { resource89C->Release(); resource89C = NULL; }
	if (resource8A0) { resource8A0->Release(); resource8A0 = NULL; }
	if (automapRenderTarget) { automapRenderTarget->Release(); automapRenderTarget = NULL; }
	if (automapDepthSurface) { automapDepthSurface->Release(); automapDepthSurface = NULL; }
	if (fogOfWarTarget) { fogOfWarTarget->Release(); fogOfWarTarget = NULL; }
	if (projectedShadowTarget) { projectedShadowTarget->Release(); projectedShadowTarget = NULL; }
	if (textureF4) { DeleteTexture(textureF4); textureF4 = 0; }
	if (projectedShadowTexture) { DeleteTexture(projectedShadowTexture); projectedShadowTexture = 0; }
	if (automapMaskHandle) { DeleteTexture(automapMaskHandle); automapMaskHandle = 0; }
	if (visibleMaskHandle) { DeleteTexture(visibleMaskHandle); visibleMaskHandle = 0; }
	if (defaultViewport) { delete defaultViewport; defaultViewport = NULL; }
	if (postViewport) { delete postViewport; postViewport = NULL; }
	Gfx_DeleteBuiltinVertexShaders();
	Gfx_ReleaseRenderStyles();
	statisticsFont.InvalidateDeviceObjects();
	statisticsFont.DeleteDeviceObjects();
	g_TextureManager->Clear();
	g_BufferManager->ReleaseAll(1);
	g_D3D.ReleaseDevice();
	// currentViewport, window, automapTexture and the borrowed list are not
	// cleared here. In particular currentViewport may now be dangling, as in
	// the original; this method does not repair caller lifetime or device state.
}

void Gfx_Engine::ResetParameters84And88()
{
	struct Parameters { float first, second; };
	static const Parameters defaults = { 0.15f, 1.5f };
	const Parameters values = defaults;
	std::memcpy(&parameter84, &values, sizeof(values));
}

DWORD Gfx_Engine::DeleteTexture(DWORD handle)
{
	//5C0910..5C091A: load actual singleton70B200, tail-jump520AB0.
	return g_TextureManager->Release(handle);
}

void Gfx_Engine::SetProjectedShadowPointCapacity(UINT count)
{
	if (shadowPoints)
	{
		::operator delete[](shadowPoints);
		shadowPoints = NULL;
	}
	shadowPointCapacity = count;
	// Actual5BDF80 allocates raw12*count bytes without initializing records.
	// The UINT multiplication preserves the original32-bit wrapped size.
	shadowPoints = static_cast<Gfx_ShadowPoint*>(::operator new[](12u * count));
	shadowPointCount = 0;
}
