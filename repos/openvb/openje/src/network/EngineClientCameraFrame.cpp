#include "EngineClient.hpp"
#include "CommandProcessor.hpp"
#include "graphics/CameraControl.hpp"
#include "graphics/ClientCursor.hpp"
#include "gui/FadeManager.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/Camera.hpp"
#include "graphics/ViewportState.hpp"
#include <stdexcept>

namespace JE
{

	void Client::DragInputCamera(LONG dx, LONG dy) //589130.
	{
		if (!field0x110 || field0xD4) return;
		D3DXVECTOR3 target(0, 0, 0);
		child0x84->GetTarget(target);
		D3DXMATRIX result, toOrigin, back, rotation;
		D3DXMatrixIdentity(&result);
		D3DXMatrixIdentity(&toOrigin);
		D3DXMatrixIdentity(&back);
		D3DXMatrixIdentity(&rotation);
		D3DXMatrixTranslation(&result, target.x - dx, target.y, target.z + dy);
		D3DXMatrixTranslation(&toOrigin, -target.x, -target.y, -target.z);
		D3DXMatrixTranslation(&back, target.x, target.y, target.z);
		float angle = child0x84->orbit.azimuth * 57.295776f;
		angle *= 0.017453292f;
		D3DXMatrixRotationY(&rotation, angle);
		D3DXMatrixMultiply(&result, &result, &toOrigin);
		D3DXMatrixMultiply(&result, &result, &rotation);
		D3DXMatrixMultiply(&result, &result, &back);
		const float delay = g_GfxSystem->Camera()->fieldOfViewY * 57.295776f - 6.0f;
		if (static_cast<LONG>(field0x114) < delay)
		{
			field0x114 = static_cast<LONG>(delay); //589538 precedes the camera callback.
			child0x84->SetTarget(D3DXVECTOR3(result._41, result._42, result._43));
		}
		else --field0x114; //Observed DWORD wrap, interpreted as signed at comparison.
	}

	BYTE Client::HandleInputEvent(const Input::InputEvent& event) //58A4A0.
	{
		if (!field0x110) return 0;
		if (event.category != 2) return 0;
		if (!event.subtype)
		{
			if (field0xD2 == 1) DragInputCamera(event.deltaX, event.deltaY);
			if (field0xD4)
			{
				if (event.deltaX)
				{
					CameraControl* camera = child0x84;
					float step = camera->azimuthSpeed * 0.00039999999f;
					step *= 33.0f;
					if (event.deltaX > 0)
						camera->SetAzimuth(camera->orbit.GetAzimuth() * 57.295776f + step);
					else if (event.deltaX < 0)
						camera->SetAzimuth(camera->orbit.GetAzimuth() * 57.295776f - step);
				}
				if (event.deltaY)
				{
					CameraControl* camera = child0x84;
					float step = camera->elevationSpeed * 0.00039999999f;
					step *= 33.0f;
					if (event.deltaY > 0)
						camera->SetElevation(camera->orbit.GetElevation() * 57.295776f - step);
					else if (event.deltaY < 0)
						camera->SetElevation(camera->orbit.GetElevation() * 57.295776f + step);
				}
				return 1;
			}
		}
		if (!field0xC0 && event.subtype == 4)
		{
			CameraControl* camera = child0x84;
			float step = camera->fieldOfViewSpeed * 0.0024999999f;
			step *= 33.0f;
			child0x84->SetFieldOfView(event.wheel < 0
				? camera->GetFieldOfView() + step : camera->GetFieldOfView() - step);
		}
		return 0;
	}

	void Client::UpdateInputCursor() //588FE0.
	{
		if (!field0x110 || field0xD4) return;
		POINT point;
		if (!::GetCursorPos(&point)) return;
		const LONG x = point.x, y = point.y;
		DWORD retained = child0x84->movementFlags & 0xFFF0;
		LONG width, height;
		if (g_GfxSystem->Windowed())
		{
			width = GetSystemMetrics(SM_CXSCREEN);
			height = GetSystemMetrics(SM_CYSCREEN);
		}
		else child0x74->GetScreenDimensions(width, height);
		LONG borderWidth, borderHeight;
		BYTE edge = 0;
		child0x84->GetScrollBorders(borderWidth, borderHeight);
		DWORD flags = 0;
		if (x >= 0 && x < borderWidth) { flags = 1; edge = 1; }
		if (x >= width - borderWidth && x < width) { flags |= 2; edge = 1; }
		if (y >= 0 && y < borderHeight) { flags |= 4; edge = 1; }
		if (y >= height - borderHeight && y < height) { flags |= 8; edge = 1; }
		if (edge == 1) retained |= flags;
		else retained |= field0x112;
		child0x84->movementFlags = static_cast<WORD>(retained);
	}

	void Client::UpdateCommandCamera()
	{
		child0x7C->Dispatch(); //587FD5, commands queued by focus/input run here.
		child0x84->Update(); //587FE0, preserve the original order.
	}
}
