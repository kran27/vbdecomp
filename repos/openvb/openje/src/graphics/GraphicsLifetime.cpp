#include <winsock2.h>
#include "Graphics.hpp"
#include "EngineFactory.hpp"
#include "MapMaskManager.hpp"
#include "OctreeRender.hpp"
#include "OctreeResourceCallbacks.hpp"
#include "SystemInterface.hpp"
#include "gui/Manager.hpp"
#include "network/EngineClient.hpp"
#include "resources/ResourceSystem.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace
{
	size_t __cdecl ReadOctreeResource(void* output, size_t size, size_t count,
		FILE* handle)
	{
		//580DA0: multiplication wraps at the actual x86 size_t/DWORD boundary.
		//Reload the published ResourceSystem on every invocation.
		return JE::Resources::g_pResourceSystem->Read(output,
			static_cast<DWORD>(size * count), reinterpret_cast<JE::Resources::ResourceFileHandle*>(handle)) ? count : 0;
	}

	int __cdecl SeekOctreeResource(FILE* handle,
		LONG offset, int origin)
	{
		//580DD0 maps every nonzero CRT origin to the resource's relative seek.
		return !JE::Resources::g_pResourceSystem->Seek(reinterpret_cast<JE::Resources::ResourceFileHandle*>(handle), offset, origin != 0);
	}

	LONG __cdecl TellOctreeResource(FILE* handle)
	{
		//580E00 returns the ResourceSystem's DWORD result without changing bits.
		return static_cast<LONG>(JE::Resources::g_pResourceSystem->Tell(reinterpret_cast<JE::Resources::ResourceFileHandle*>(handle)));
	}

	DWORD __cdecl GraphicsRet0(DWORD)
	{
		//580EC6 calls4080B0 with0: the genuine body is XOR EAX,EAX; RET.
		return 0;
	}

	void GraphicsPostRelease()
	{
		//583897 calls404B70: the genuine body is RET and reads no receiver.
	}
}

Graphics::Graphics()
{
	initialized = 0;
	backend = NULL;
	cameraControl = NULL;
	renderer = NULL;
	maskManager = NULL;
	liveFullscreen = 1;
	liveColor32 = 0;
	liveWidth = 800;
	liveHeight = 600;
	liveRefresh = 0;
	adapter = 0;
	device = 0;
	displayMode = 0;
	vsync = 1;
	statistics = 0;
	clearRed = 0;
	clearGreen = 64;
	clearBlue = 64;
	shadowMode = 0;
	lighting = 1;
	linearFiltering = 1;
	startupMovies = 1;
	flag57 = 1;
	wireframe = 0;
	transparency = 1;
	flag5A = 0;
	flag5B = 0;
	sceneEnabled = 1;
	bounds = 0;
	debugBounds = 0;
	currentOctree = NULL;
	//value28, the requested display fields and ordinary padding stay unwritten.
	SetOctreeResourceCallbacks(ReadOctreeResource, SeekOctreeResource,
		TellOctreeResource);
	GraphicsRet0(0);
	backend = Gfx_CreateEngine();
	if (!backend)
		JE::FatalError("Unable to create graphics system.");
	JE::LogDebugString("Graphics system created.\n");
	cameraControl = NULL;
	renderer = new Octree2Gfx;
	previousClock = JE::EngineClientServices::AccumulatedTime();
	maskManager = new MapMaskManager;
}

Graphics::~Graphics()
{
	WriteSettings();
	delete maskManager;
	//58386F calls Cleanup unconditionally, then reloads renderer for deletion.
	//Its actual destructor calls Cleanup again; neither member is reset here.
	renderer->Cleanup();
	delete renderer;
	Cleanup();
	Gfx_ReleaseEngine();
	GraphicsPostRelease();
	JE::LogDebugString("Graphics system destroyed.\n");
}

void Graphics::Cleanup()
{
	initialized = 0;
	::GUI::g_Manager.Cleanup();
	//58149E reads the published global only after Manager::Cleanup returns.
	//It does not dispatch through this object's saved backend member.
	if (g_GfxSystem)
		g_GfxSystem->ShutdownDevice();
}

Graphics* Gfx_CreateGraphics()
{
	return new Graphics;
}
