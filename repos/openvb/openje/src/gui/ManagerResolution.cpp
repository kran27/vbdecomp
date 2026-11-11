#include "gui/Manager.hpp"
#include "gui/Interface.hpp"
#include "graphics/Viewport.hpp"
#include "graphics/CameraController.hpp"

namespace GUI
{
	void Manager::RefreshPositions()
	{
		for (std::list<Interface*>::iterator entry = interfaces.begin(); entry != interfaces.end(); ++entry)
			(*entry)->RefreshPositions();
	}

	void Manager::SetResolution(LONG width, LONG height)
	{
		DWORD selectedWidth = static_cast<DWORD>(width);
		DWORD selectedHeight = static_cast<DWORD>(height);
		// Original FILD operands are signed, while the stored dimensions and
		// D3D viewport fields retain all raw input bits.
		scaleX = width * 0.0009765625f;
		resolutionWidth = static_cast<DWORD>(width);
		resolutionHeight = static_cast<DWORD>(height);
		scaleY = height * 0.0013020833721384406f;
		Gfx_Viewport* current = view;
		if (current)
		{
			if (!width) selectedWidth = current->viewport.Width;
			if (!height) selectedHeight = current->viewport.Height;
			D3DVIEWPORT8 descriptor;
			descriptor.Width = selectedWidth;
			descriptor.X = 0; descriptor.Y = 0;
			descriptor.Height = selectedHeight;
			descriptor.MinZ = 0.0f; descriptor.MaxZ = 1.0f;
			current->SetViewport(descriptor);
			current = view; //Reload the owner after the actual viewport/mesh child.
			current->clearColor = 0;
			current->vertices[3].diffuse = 0;
			current->vertices[2].diffuse = 0;
			current->vertices[1].diffuse = 0;
			current->vertices[0].diffuse = 0;
			view->SetCameraController(camera);
		}
	}
}
