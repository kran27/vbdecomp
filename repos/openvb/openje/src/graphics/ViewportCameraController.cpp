#include "Viewport.hpp"
#include "CameraController.hpp"

void Gfx_Viewport::SetCameraController(Gfx_CamCtrl* controller)
{
	if (controller)
	{
		if (activeCamera)
		{
			Gfx_Camera* camera = activeCamera;
			controller->cameras[camera] = 0;
		}
		controller->dirty = 1;
	}
	else
	{
		if (cameraController)
		{
			cameraController->RemoveCamera(&camera);
			if (activeCamera != &camera)
			{
				cameraController->RemoveCamera(activeCamera);
			}
		}
	}
	cameraController = controller;
}
