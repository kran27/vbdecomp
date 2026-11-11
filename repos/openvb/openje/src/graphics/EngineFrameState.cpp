#include "EngineFrameState.hpp"
#include "Viewport.hpp"
#include "RenderManager.hpp"
#include "D3DState.hpp"
#include "diagnostics/DebugOutput.hpp"

Gfx_EngineFrameState::Gfx_EngineFrameState() : deviceState(0), currentViewport(NULL),
	defaultViewport(NULL), postViewport(NULL), window(NULL), automapRequested(0), automapReady(1),
	projectedShadows(0), statistics(0), fogOfWar(0)
{
}
Gfx_EngineFrameView::Gfx_EngineFrameView(Gfx_EngineFrameState& state, BYTE& debug)
	: deviceState(state.deviceState), currentViewport(state.currentViewport), defaultViewport(state.defaultViewport),
	postViewport(state.postViewport), additionalViewports(state.additionalViewports), window(state.window),
	automapRequested(state.automapRequested), automapReady(state.automapReady), projectedShadows(state.projectedShadows),
	statistics(state.statistics), fogOfWar(state.fogOfWar), debugFrames(debug)
{
}
Gfx_EngineFrameView::Gfx_EngineFrameView(DWORD& lost, Gfx_Viewport*& current,
	Gfx_Viewport*& primary, Gfx_Viewport*& post, std::list<Gfx_Viewport*>& additional,
	HWND& outputWindow, BYTE& requested, BYTE& ready, BYTE& shadows, BYTE& stats, BYTE& fog, BYTE& debug)
	: deviceState(lost), currentViewport(current), defaultViewport(primary), postViewport(post),
	additionalViewports(additional), window(outputWindow), automapRequested(requested), automapReady(ready),
	projectedShadows(shadows), statistics(stats), fogOfWar(fog), debugFrames(debug)
{
}

void Gfx_ConfigureEngineViewports(Gfx_EngineFrameView& state, const D3DVIEWPORT8& descriptor, HWND window)
{
	if (!state.defaultViewport) JE::FatalError("Gfx_Engine::CreateDevice - Failed to create Default Viewport");
	state.defaultViewport->SetViewport(descriptor);
	Gfx_Viewport* selected = state.defaultViewport;
	selected->clearColor = 0xFF002080;
	for (int i = 3; i >= 0; --i) selected->vertices[i].diffuse = 0xFF002080;
	state.defaultViewport->clearFlags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER;
	state.currentViewport = state.defaultViewport;
	if (!state.postViewport) JE::FatalError("Gfx_Engine::CreateDevice - Failed to create PostRenderViewport");
	state.postViewport->SetViewport(descriptor);
	state.postViewport->clearFlags = D3DCLEAR_ZBUFFER;
	selected = state.postViewport;
	selected->clearColor = 0;
	for (int i = 3; i >= 0; --i) selected->vertices[i].diffuse = 0;
	state.window = window;
}

bool Gfx_RenderEngineFrame(Gfx_EngineFrameView& state, Gfx_EngineFrameServices& services,
	BYTE clearQueues, BYTE present)
{
	IDirect3DDevice8* beginDevice = g_D3D.device;
	++g_D3D.statistics[6];
	if (FAILED(beginDevice->BeginScene())) return false;
	void (WINAPI* const outputDebug)(LPCSTR) = OutputDebugStringA;
	if (state.debugFrames) outputDebug("\n---BeginScene\n");
	Gfx_Viewport* selected = state.defaultViewport;
	state.currentViewport = selected;
	g_D3D.device->SetViewport(&selected->viewport);
	state.defaultViewport->Render(clearQueues);
	std::list<Gfx_Viewport*>::iterator current = state.additionalViewports.begin();
	while (current != state.additionalViewports.end())
	{
		selected = *current;
		state.currentViewport = selected;
		g_D3D.device->SetViewport(&selected->viewport);
		state.currentViewport->Render(clearQueues);
		++current;
	}
	selected = state.postViewport;
	state.currentViewport = selected;
	g_D3D.device->SetViewport(&selected->viewport);
	state.postViewport->Render(clearQueues);
	if (state.statistics)
	{
		g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_D3D.device->SetViewport(&state.defaultViewport->viewport);
		services.RenderStatistics();
	}
	++g_D3D.statistics[7];
	g_D3D.device->EndScene();
	if (state.debugFrames) outputDebug("---EndScene\n");
	if (present)
	{
		const HRESULT result = g_D3D.device->Present(0, 0, state.window, 0);
		if (FAILED(result))
			if (result == D3DERR_DEVICELOST) state.deviceState = 1;
	}
	g_D3D.UpdateStatistics();
	return true;
}

void Gfx_UpdateEngineFrame(Gfx_EngineFrameView& state, Gfx_EngineUpdateServices& services)
{
	if (!g_D3D.device) return;
	if (g_D3D.device->TestCooperativeLevel() == D3DERR_DEVICELOST) state.deviceState = 1;
	if (state.deviceState != 1)
	{
		if (state.fogOfWar) services.ProcessFogOfWar();
		if (state.projectedShadows) services.ProjectShadows();
		if (!state.automapRequested) Gfx_RenderEngineFrame(state, services, 1, 1);
		else
		{
			services.RenderAutomap();
			state.automapRequested = 0;
			state.automapReady = 1;
		}
		state.additionalViewports.clear();
		return;
	}
	const HRESULT cooperativeResult = g_D3D.device->TestCooperativeLevel();
	if (cooperativeResult == D3DERR_DEVICENOTRESET)
	{
		JE::LogDebugString("Release Video Resources\n");
		services.ReleaseVideoResources();
		JE::LogDebugString("Attempt to Reset Device\n");
		const HRESULT resetResult = g_D3D.ResetDevice(0);
		if (resetResult) JE::LogDebugString("DeviceReset() failed: %x\n", resetResult);
		else
		{
			services.RestoreVideoResources();
			state.defaultViewport->renderer->Clear();
			state.postViewport->renderer->Clear();
			for (std::list<Gfx_Viewport*>::iterator viewport = state.additionalViewports.begin();
				viewport != state.additionalViewports.end(); ++viewport) (*viewport)->renderer->Clear();
			state.deviceState = 0;
			JE::LogDebugString("Back to normal\n");
		}
	}
	else if (cooperativeResult)
	{
		state.defaultViewport->renderer->Clear();
		state.postViewport->renderer->Clear();
		for (std::list<Gfx_Viewport*>::iterator viewport = state.additionalViewports.begin();
			viewport != state.additionalViewports.end(); ++viewport) (*viewport)->renderer->Clear();
		Sleep(1000);
	}
	else
	{
		JE::LogDebugString("TestCooperativeLevel() failed: %x\n", cooperativeResult);
		state.deviceState = 0;
	}
}
