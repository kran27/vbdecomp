#include "game/POIWaypoint.hpp"
#include "gui/LineSystem.hpp"
#include <cstdlib>
#include <new>

POIWaypoint::POIWaypoint() : GUI::Label(3), valueE6C(0.0f), valueE70(0.0f), valueE74(0.0f)
{
	coordinateX = 0;
	coordinateY = 0;
	valueE78 = 0.0f;
	lineSystem = new GUI::LineSystem;
}

POIWaypoint::~POIWaypoint()
{
	delete lineSystem;
}

void POIWaypoint::Refresh()
{
	GUI::Window::Refresh();
	if (lineSystem)
	{
		lineSystem->Update();
	}
}

void POIWaypoint::Draw()
{
	GUI::Label::Draw();
	if (lineSystem)
	{
		lineSystem->Draw(GetViewport());
	}
}

float POIWaypoint::SetDepth(float depth, float increment)
{
	float saved = GUI::Label::SetDepth(depth, increment); //458993 stores float32.
	if (lineSystem)
	{
		lineSystem->SetDepth(saved, increment);
	}
	return saved;
}
