#include "CameraControl.hpp"
#include <cmath>

namespace JE
{
	void CameraControl::SetTargetDistance(float value) //586140.
	{
		float target = value;
		float current = orbit.GetDistance();
		//586164 tests C0|C2 then JP: unordered takes the upper-bound test.
		//Spelling this as !(value >= minimumDistance) incorrectly clamps NaN.
		if (value < minimumDistance) target = minimumDistance;
		else if (value > maximumDistance) target = maximumDistance;
		//58619F stores the current ceil as float before comparing target ceil.
		float roundedCurrent = float(std::ceil(double(current)));
		if (float(std::ceil(double(target))) == roundedCurrent)
		{
			orbit.SetOrbit(orbit.GetAzimuth(), orbit.GetElevation(), target);
			transitionFlags &= ~WORD(4);
		}
		else
		{
			if (target > current) distanceDirection = 1.0f;
			else distanceDirection = -1.0f;
			transitionFlags |= 4;
			targetDistance = target;
		}
	}
	void CameraControl::ToggleTopDown(BYTE transition)
	{
		float elevation = 0.0f;
		if (!(orbit.GetElevation() * 57.295776f > 45.0f)) elevation = 90.0f;
		if (transition) SetTargetElevation(elevation);
		else SetElevation(elevation);
	}

	void CameraControl::Top(BYTE transition)
	{
		if (transition) SetTargetElevation(90.0f);
		else SetElevation(90.0f);
	}

	void CameraControl::Down(BYTE transition)
	{
		if (transition) SetTargetElevation(0.0f);
		else SetElevation(0.0f);
	}

}
