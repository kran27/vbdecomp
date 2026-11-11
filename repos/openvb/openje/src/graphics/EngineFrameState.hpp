#pragma once

#include <windows.h>
#include <d3d8.h>
#include <list>

class Gfx_Viewport;

//Only5BFCB0/5C1940 fields, with actual constructor defaults. Normal list
//nodes own borrowed viewport pointers; this state is not a Gfx_Engine.
struct Gfx_EngineFrameState
{
	Gfx_EngineFrameState();
	DWORD deviceState;
	Gfx_Viewport* currentViewport;
	Gfx_Viewport* defaultViewport;
	Gfx_Viewport* postViewport;
	std::list<Gfx_Viewport*> additionalViewports;
	HWND window;
	BYTE automapRequested, automapReady, projectedShadows, statistics, fogOfWar;
private:
	Gfx_EngineFrameState(const Gfx_EngineFrameState&);
	Gfx_EngineFrameState& operator=(const Gfx_EngineFrameState&);
};

//Live aliases preserve pointer/flag reloads across SDK and game callbacks.
struct Gfx_EngineFrameView
{
	Gfx_EngineFrameView(Gfx_EngineFrameState& state, BYTE& debug);
	Gfx_EngineFrameView(DWORD& deviceState, Gfx_Viewport*& currentViewport,
		Gfx_Viewport*& defaultViewport, Gfx_Viewport*& postViewport,
		std::list<Gfx_Viewport*>& additionalViewports, HWND& window,
		BYTE& automapRequested, BYTE& automapReady, BYTE& projectedShadows,
		BYTE& statistics, BYTE& fogOfWar, BYTE& debugFrames);
	DWORD& deviceState;
	Gfx_Viewport*& currentViewport;
	Gfx_Viewport*& defaultViewport;
	Gfx_Viewport*& postViewport;
	std::list<Gfx_Viewport*>& additionalViewports;
	HWND& window;
	BYTE& automapRequested;
	BYTE& automapReady;
	BYTE& projectedShadows;
	BYTE& statistics;
	BYTE& fogOfWar;
	BYTE& debugFrames;
};

class Gfx_EngineFrameServices
{
public:
	virtual ~Gfx_EngineFrameServices() {}
	virtual void RenderStatistics() = 0; //5BF9E0, actual owner or addressed boundary.
};
class Gfx_EngineUpdateServices : public Gfx_EngineFrameServices
{
public:
	virtual void ProcessFogOfWar() = 0; //5BE380.
	virtual void ProjectShadows() = 0; //5BEF80.
	virtual void RenderAutomap() = 0; //5C1000.
	virtual void ReleaseVideoResources() = 0; //5BEDF0.
	virtual void RestoreVideoResources() = 0; //5BEF20.
};

bool Gfx_RenderEngineFrame(Gfx_EngineFrameView& state, Gfx_EngineFrameServices& services,
	BYTE clearQueues, BYTE present); //5BFCB0.
void Gfx_UpdateEngineFrame(Gfx_EngineFrameView& state, Gfx_EngineUpdateServices& services); //5C1940.
void Gfx_ConfigureEngineViewports(Gfx_EngineFrameView& state, const D3DVIEWPORT8& descriptor,
	HWND window); //5C0D19..5C0DE6, after both actual viewport constructors.
