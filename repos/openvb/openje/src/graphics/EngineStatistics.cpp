#include "Engine.hpp"
#include "D3DState.hpp"
#include "Camera.hpp"
#include "CameraController.hpp"
#include <cstdio>

void Gfx_Engine::RenderStatistics()
{
	if (!mapName.empty())
	{
		std::sprintf(statisticsText, "Map: %s", mapName.c_str());
		statisticsFont.DrawText(2.0f, 0.0f, 0xFFFFFF80u, statisticsText, 0);
	}

	std::sprintf(statisticsText, "FPS: %3.1f V: %d T: %d Vis: %d",
		g_D3D.framesPerSecond, g_D3D.statistics[4], g_D3D.statistics[3],
		g_D3D.statistics[13]);
	statisticsFont.DrawText(2.0f, 20.0f, 0xFFFFFF80u, statisticsText, 0);

	const char* light = lighting ? "On" : "Off";
	const char* trans = transparency ? "On" : "Off";
	std::sprintf(statisticsText,
		"Tex: %d VB: %d Mtx: %d DIP: %d DP: %d Trans: %s Light: %s",
		g_D3D.statistics[8], g_D3D.statistics[9], g_D3D.statistics[10],
		g_D3D.statistics[12], g_D3D.statistics[11], trans, light);
	statisticsFont.DrawText(2.0f, 40.0f, 0xFFFFFF80u, statisticsText, 0);

	Gfx_Camera* camera = Camera();
	Gfx_CamCtrl* control = CameraController();
	if (camera)
	{
		std::sprintf(statisticsText, "Cam: (%4.1f, %4.1f, %4.1f)",
			camera->transform._41, camera->transform._42, camera->transform._43);
		statisticsFont.DrawText(2.0f, 60.0f, 0xFFFFFF80u, statisticsText, 0);
	}
	if (control)
	{
		Gfx_CamCtrl_Orbit* orbit = dynamic_cast<Gfx_CamCtrl_Orbit*>(control);
		if (camera && orbit)
		{
			const D3DXVECTOR3 target = orbit->target;
			std::sprintf(statisticsText, "Target: (%4.1f, %4.1f, %4.1f)",
				target.x, target.y, target.z);
			statisticsFont.DrawText(2.0f, 80.0f, 0xFFFFFF80u, statisticsText, 0);

			//Original647920 is float bits42652EE0. The products go directly to
			//the CRT's promoted double arguments, without an intervening float store.
			const float radiansToDegrees = 57.2957763671875f;
			std::sprintf(statisticsText, "Azi: %3.1f Elev: %3.1f Dist: %4.1f FOVy: %3.1f",
				orbit->azimuth * radiansToDegrees,
				orbit->elevation * radiansToDegrees, orbit->distance,
				camera->fieldOfViewY * radiansToDegrees);
			statisticsFont.DrawText(2.0f, 100.0f, 0xFFFFFF80u, statisticsText, 0);
		}
	}
}
