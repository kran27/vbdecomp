#include "Engine.hpp"
#include "Viewport.hpp"
#include "D3DState.hpp"

BYTE g_GfxDebugFrames; //Original70C968; canonical static-zero global, shared by real Engines.

Gfx_EngineFrameView Gfx_Engine::FrameView()
{
	return Gfx_EngineFrameView(value04, currentViewport, defaultViewport, postViewport,
		additionalViewports, window, automapRequested, automapReady, projectedShadows,
		statistics, fogOfWar, g_GfxDebugFrames);
}
bool Gfx_Engine::RenderFrame(BYTE clearQueues, BYTE present)
{
	IDirect3DDevice8* beginDevice = g_D3D.device;
	bool result = false;
	++g_D3D.statistics[6];
	if (SUCCEEDED(beginDevice->BeginScene()))
	{
		void (WINAPI* const outputDebug)(LPCSTR) = OutputDebugStringA;
		if (g_GfxDebugFrames) outputDebug("\n---BeginScene\n");
		Gfx_Viewport* selected = defaultViewport;
		currentViewport = selected;
		g_D3D.device->SetViewport(&selected->viewport);
		defaultViewport->Render(clearQueues);
		std::list<Gfx_Viewport*>::iterator current = additionalViewports.begin();
		if (current != additionalViewports.end())
		{
			do
			{
				selected = *current;
				currentViewport = selected;
				g_D3D.device->SetViewport(&selected->viewport);
				currentViewport->Render(clearQueues);
				++current;
			} while (current != additionalViewports.end());
		}
		selected = postViewport;
		currentViewport = selected;
		g_D3D.device->SetViewport(&selected->viewport);
		postViewport->Render(clearQueues);
		if (statistics)
		{
			g_D3D.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			g_D3D.device->SetViewport(&defaultViewport->viewport);
			RenderStatistics();
		}
		++g_D3D.statistics[7];
		g_D3D.device->EndScene();
		if (g_GfxDebugFrames) outputDebug("---EndScene\n");
		if (present)
		{
			const HRESULT result = g_D3D.device->Present(0, 0, window, 0);
			if (FAILED(result))
				if (result == D3DERR_DEVICELOST) value04 = 1;
		}
		g_D3D.UpdateStatistics();
		result = true;
	}
	return result;
}
