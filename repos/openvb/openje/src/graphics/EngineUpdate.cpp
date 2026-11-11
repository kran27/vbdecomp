#include "Engine.hpp"
#include "Viewport.hpp"
#include "RenderManager.hpp"
#include "D3DState.hpp"
#include "../diagnostics/DebugOutput.hpp"

void Gfx_Engine::RenderFrame()
{
	if (!g_D3D.device) return;
	if (g_D3D.device->TestCooperativeLevel() == D3DERR_DEVICELOST) value04 = 1;
	if (value04 != 1)
	{
		if (fogOfWar) ProcessFogOfWar();
		if (projectedShadows) ProjectShadows();
		if (!automapRequested) RenderFrame(1, 1);
		else
		{
			RenderAutomap();
			automapRequested = 0;
			automapReady = 1;
		}
		additionalViewports.clear();
		return;
	}
	const HRESULT cooperativeResult = g_D3D.device->TestCooperativeLevel();
	if (cooperativeResult == D3DERR_DEVICENOTRESET)
	{
		JE::LogDebugString("Release Video Resources\n");
		ReleaseVideoResources();
		JE::LogDebugString("Attempt to Reset Device\n");
		const HRESULT resetResult = g_D3D.ResetDevice(0);
		if (resetResult) JE::LogDebugString("DeviceReset() failed: %x\n", resetResult);
		else
		{
			RestoreVideoResources();
			defaultViewport->renderer->Clear();
			postViewport->renderer->Clear();
			for (std::list<Gfx_Viewport*>::iterator viewport = additionalViewports.begin();
				viewport != additionalViewports.end(); ++viewport) (*viewport)->renderer->Clear();
			value04 = 0;
			JE::LogDebugString("Back to normal\n");
		}
	}
	else if (cooperativeResult)
	{
		defaultViewport->renderer->Clear();
		postViewport->renderer->Clear();
		for (std::list<Gfx_Viewport*>::iterator viewport = additionalViewports.begin();
			viewport != additionalViewports.end(); ++viewport) (*viewport)->renderer->Clear();
		Sleep(1000);
	}
	else
	{
		JE::LogDebugString("TestCooperativeLevel() failed: %x\n", cooperativeResult);
		value04 = 0;
	}
}
