#include "gui/DialogueWindow.hpp"
#include "gui/Manager.hpp"
#include "graphics/Viewport.hpp"
#include "graphics/CameraController.hpp"
#include "graphics/ModelInstance.hpp"
#include "graphics/AnimationObject.hpp"
#include "graphics/AnimationInstance.hpp"
#include "graphics/RenderManager.hpp"
#include "graphics/Light.hpp"
#include <mmsystem.h>

namespace Granny21
{
	extern "C" __declspec(dllimport) int __stdcall GrannyGetControlLoopCount(Control* control);
}

namespace JE
{
	void DialogueWindow::UpdatePortrait() //591C60.
	{
		const DWORD now = timeGetTime();
		if (portraitAnimation)
		{
			BYTE restart = !portraitAnimation->playing;
			if (!restart)
			{
				Granny21::Control* control = portraitAnimation->control;
				if (!control) control = portraitAnimation->nextControl;
				if (control)
				{
					const LONG loops = Granny21::GrannyGetControlLoopCount(control);
					if (loops > 0)
					{
						const float duration = portraitAnimation->animationObject->GetDuration(
							portraitAnimation->group, portraitAnimation->animationIndex, portraitAnimation->value54);
						restart = float(loops) * duration <= float(DWORD(now - animationTimeOrigin)) * 0.001f;
					}
				}
			}
			if (restart)
			{
				portraitAnimation->SelectAnimation(1, 0, 0, 0.1f, 1, 0);
				portraitAnimation->SetSpeed(1.0f);
				portraitAnimation->reverse = 0;
				animationTimeOrigin = timeGetTime();
			}
			portraitAnimation->Update(float(DWORD(now - animationTimeOrigin)) * 0.001f);
		}
		if (portraitModel && portraitViewport)
		{
			::GUI::g_Manager.AddViewport(portraitViewport);
			D3DXVECTOR3 target(portraitModel->transform._41,
				portraitModel->transform._42 + 2.8f, portraitModel->transform._43);
			Gfx_CamCtrl_Orbit* orbit = portraitOrbit;
			orbit->target.x = target.x;
			orbit->dirty = 1;
			orbit->target.y = target.y;
			orbit->target.z = target.z;
			const float elevation = portraitElevation * 0.017453292f;
			portraitOrbit->SetOrbit(portraitAzimuth, elevation, portraitDistance);
			portraitOrbit->Update();
			Gfx_Viewport* viewport = portraitViewport;
			viewport->flag1C = 1;
			viewport->renderer->QueueMeshInstances(&viewport->instance, 1);
		}
	}
	void DialogueWindow::QueuePortrait() //591E40.
	{
		if (portraitModel && portraitViewport)
		{
			portraitModel->ClearLights();
			if (portraitLight) portraitModel->AddLight(portraitLight);
			portraitModel->Submit(portraitViewport, 0, 0.05f, 0xFFFF0000);
		}
	}
}

