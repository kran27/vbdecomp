#include <winsock2.h>
#include "CameraControl.hpp"
#include "Camera.hpp"
#include "SystemInterface.hpp"
#include "Graphics.hpp"
#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "startup/EngineSettings.hpp"
#include "startup/MessageLoop.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstdlib>
#include <cmath>
#include <stdexcept>

namespace
{
	void ReadFloat(const JE::Configuration::Values& values, const char* name, float& output)
	{
		JE::Configuration::Values::const_iterator found = values.find(name);
		if (found != values.end())
		{
			output = static_cast<float>(std::atof(found->second.c_str()));
		}
	}




}

namespace JE
{

	CameraControl::CameraControl(Client* owner) : client(owner), transitionFlags(0),
		movementFlags(0), lastUpdateTime(0), constraints(0)
	{
		savedNearPlane = g_GfxSystem->Camera()->nearPlane;
		savedFarPlane = g_GfxSystem->Camera()->farPlane;
		client->child0x74->AttachCamera(&orbit);
		Reset();
		lastUpdateTime = F3::GetElapsedTimeMS();
	}

	CameraControl::~CameraControl()
	{
		//Ordinary embedded Orbit/map destruction; borrowed cameras are not detached.
	}

	void CameraControl::OnCleanup()
	{
		//Original shared404B70 body.
	}

	void CameraControl::LoadConfiguration()
	{
		azimuthSpeed = 360.0f;
		elevationSpeed = 360.0f;
		minimumElevation = 38.5f;
		maximumElevation = 89.0f;
		distanceSpeed = 800.0f;
		minimumDistance = maximumDistance = 350.0f;
		fieldOfViewSpeed = 6.5f;
		minimumFieldOfView = 6.0f;
		maximumFieldOfView = 15.0f;
		scrollBorder = 0.01f;
		scrollSpeed = 96.0f;
		const Configuration::Settings& settings = *F3::GetEngineSettings();
		Configuration::Settings::const_iterator section = settings.find("Camera");
		if (section != settings.end())
		{
			const Configuration::Values& values = section->second;
			{
				Configuration::Values::const_iterator found = values.find("Azimuth Speed");
				if (found != values.end())
					azimuthSpeed = static_cast<float>(std::atof(found->second.c_str()));
			}
			{
				Configuration::Values::const_iterator found = values.find("Elevation Speed");
				if (found != values.end())
					elevationSpeed = static_cast<float>(std::atof(found->second.c_str()));
			}
			{
				Configuration::Values::const_iterator found = values.find("Elevation Min");
				if (found != values.end())
					minimumElevation = static_cast<float>(std::atof(found->second.c_str()));
			}
			{
				Configuration::Values::const_iterator found = values.find("Elevation Max");
				if (found != values.end())
					maximumElevation = static_cast<float>(std::atof(found->second.c_str()));
			}
			{
				Configuration::Values::const_iterator found = values.find("Distance Speed");
				if (found != values.end())
					distanceSpeed = static_cast<float>(std::atof(found->second.c_str()));
			}
			{
				Configuration::Values::const_iterator found = values.find("Distance Min");
				if (found != values.end())
					minimumDistance = static_cast<float>(std::atof(found->second.c_str()));
			}
			{
				Configuration::Values::const_iterator found = values.find("Distance Max");
				if (found != values.end())
					maximumDistance = static_cast<float>(std::atof(found->second.c_str()));
			}
			{
				Configuration::Values::const_iterator found = values.find("FOV Speed");
				if (found != values.end())
					fieldOfViewSpeed = static_cast<float>(std::atof(found->second.c_str()));
			}
			{
				Configuration::Values::const_iterator found = values.find("FOV Min");
				if (found != values.end())
					minimumFieldOfView = static_cast<float>(std::atof(found->second.c_str()));
			}
			{
				Configuration::Values::const_iterator found = values.find("FOV Max");
				if (found != values.end())
					maximumFieldOfView = static_cast<float>(std::atof(found->second.c_str()));
			}
			{
				Configuration::Values::const_iterator found = values.find("Scroll Speed");
				if (found != values.end())
					scrollSpeed = static_cast<float>(std::atof(found->second.c_str()));
			}
			{
				Configuration::Values::const_iterator found = values.find("Scroll Border");
				if (found != values.end())
					scrollBorder = static_cast<float>(std::atof(found->second.c_str()));
			}
		}
		Reset();
	}

	void CameraControl::GetScrollBorders(LONG& width, LONG& height) //586230.
	{
		static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x74->GetScreenDimensions(width, height);
		width = static_cast<LONG>(static_cast<DWORD>(width) * scrollBorder);
		height = static_cast<LONG>(static_cast<DWORD>(height) * scrollBorder);
	}

	void CameraControl::GetTarget(D3DXVECTOR3& output) const
	{
		const D3DXVECTOR3 target = orbit.target;
		output.x = target.x;
		output.y = target.y;
		output.z = target.z;
	}

	void CameraControl::SetAzimuth(float degrees)
	{
		transitionFlags &= ~WORD(1);
		orbit.SetOrbit(degrees * 0.017453292f, orbit.GetElevation(), orbit.GetDistance());
	}

	void CameraControl::SetElevation(float degrees)
	{
		transitionFlags &= ~WORD(2);
		orbit.SetOrbit(orbit.GetAzimuth(), degrees * 0.017453292f, orbit.GetDistance());
	}

	void CameraControl::SetTargetAzimuth(float degrees)
	{
		float current = orbit.azimuth * 57.295776f;
		float target = static_cast<float>(std::fmod(degrees, 360.0));
		float roundedCurrent = static_cast<float>(std::ceil(current));
		if (float(std::ceil(double(target))) == roundedCurrent)
		{
			orbit.SetOrbit(target * 0.017453292f, orbit.GetElevation(), orbit.GetDistance());
			transitionFlags &= ~WORD(1);
			return;
		}
		transitionFlags |= 1;
		float delta = static_cast<float>(std::fabs(target - current));
		float complement = static_cast<float>(360.0f - delta);
		if (delta <= complement)
		{
			if (current > target) azimuthDirection = -1.0f;
			else azimuthDirection = 1.0f;
		}
		else if (current > target)
		{
			azimuthDirection = 1.0f;
			target = complement + current;
		}
		else
		{
			azimuthDirection = -1.0f;
			target = current - complement;
		}
		targetAzimuth = target;
		currentAzimuth = current;
	}

	void CameraControl::SetTargetElevation(float degrees)
	{
		float target = degrees;
		float current = orbit.elevation * 57.295776f;
		//58603A/58603D preserves unordered input in the upper-bound branch.
		if (degrees < minimumElevation) target = minimumElevation;
		else if (degrees > maximumElevation) target = maximumElevation;
		float roundedCurrent = static_cast<float>(std::ceil(double(current)));
		if (float(std::ceil(double(target))) == roundedCurrent)
		{
			orbit.SetOrbit(orbit.GetAzimuth(), target * 0.017453292f, orbit.GetDistance());
			transitionFlags &= ~WORD(2);
		}
		else
		{
			if (target > current) elevationDirection = 1.0f;
			else elevationDirection = -1.0f;
			transitionFlags |= 2;
			targetElevation = target;
		}
	}

	void CameraControl::SetDistance(float value)
	{
		transitionFlags &= ~WORD(4);
		orbit.SetOrbit(orbit.GetAzimuth(), orbit.GetElevation(), value);
	}

	void CameraControl::ClampTarget(D3DXVECTOR3& target)
	{
		if (constraints)
		{
			D3DXVECTOR3 minimum(0,0,0), maximum(0,0,0);
			IWorld* world = g_World;
			world->GetCurrentMap()->GetCameraBounds(minimum, maximum);
			if (target.x < minimum.x) target.x = minimum.x;
			else if (target.x > maximum.x) target.x = maximum.x;
			//Original World API supplies descending Z bounds: first is upper Z.
			if (target.z > minimum.z) target.z = minimum.z;
			else if (target.z < maximum.z) target.z = maximum.z;
			target.y = 0.0f;
		}
	}

	void CameraControl::SetTarget(D3DXVECTOR3 value)
	{
		transitionFlags &= ~WORD(8);
		ClampTarget(value);
		D3DXVECTOR3 target(value);
		orbit.target = target;
		orbit.dirty = 1;
	}

	void CameraControl::MoveTarget(float x, float y, float z)
	{
		Gfx_CamCtrl_Orbit& controller = orbit;
		controller.TranslateTarget(x, y, z);
		D3DXVECTOR3 target = controller.target;
		D3DXVECTOR3 minimum(0,0,0), maximum(0,0,0);
		IWorld* world = g_World;
		Map* map = world->GetCurrentMap();
		if (!map)
		{
			LogDebugString("CameraControl::MoveTarget() - map is NULL!");
			return;
		}
		map->GetCameraBounds(minimum, maximum);
		BYTE changed = 0;
		if (target.x < minimum.x) { target.x = minimum.x; changed = 1; }
		else if (target.x > maximum.x) { target.x = maximum.x; changed = 1; }
		if (target.z > minimum.z) target.z = minimum.z;
		else if (target.z < maximum.z) target.z = maximum.z;
		else if (!changed) return;
		controller.target = target;
		controller.dirty = 1;
	}

	float CameraControl::GetFieldOfView() const
	{
		return g_GfxSystem->Camera()->fieldOfViewY * 57.295776f;
	}

	void CameraControl::SetConstraints(BYTE enabled)
	{
		if (constraints == enabled) return;
		constraints = enabled;
		if (enabled)
		{
			float nearValue = savedNearPlane;
			Gfx_Camera* camera = g_GfxSystem->Camera();
			camera->nearPlane = nearValue; camera->projectionDirty = 1;
			float farValue = savedFarPlane;
			camera = g_GfxSystem->Camera();
			camera->farPlane = farValue; camera->projectionDirty = 1;
			//586590 uses raw configured limits here; Reset separately converts degrees.
			if (minimumElevation >= -1.5620697736740112f) orbit.minimumElevation = minimumElevation;
			if (maximumElevation <= 1.5620697736740112f) orbit.maximumElevation = maximumElevation;
			if (minimumDistance >= 0.0f) orbit.minimumDistance = minimumDistance;
			if (maximumDistance > orbit.minimumDistance) orbit.maximumDistance = maximumDistance;
		}
		else
		{
			savedNearPlane = g_GfxSystem->Camera()->nearPlane;
			savedFarPlane = g_GfxSystem->Camera()->farPlane;
			orbit.minimumElevation = -1.5620697736740112f;
			orbit.maximumElevation = 1.5620697736740112f;
			orbit.minimumDistance = 0.0f;
			if (orbit.minimumDistance < 1000.0f) orbit.maximumDistance = 1000.0f;
		}
	}

	void CameraControl::SetProjection(float nearPlane, float farPlane)
	{
		if (!constraints)
		{
			Gfx_Camera* camera = g_GfxSystem->Camera();
			camera->nearPlane = nearPlane; camera->projectionDirty = 1;
			camera = g_GfxSystem->Camera();
			camera->farPlane = farPlane; camera->projectionDirty = 1;
		}
	}

	void CameraControl::SetFieldOfView(float degrees)
	{
		if (constraints)
		{
			if (degrees < minimumFieldOfView) degrees = minimumFieldOfView;
			else if (degrees > maximumFieldOfView) degrees = maximumFieldOfView;
		}
		Gfx_Camera* camera = g_GfxSystem->Camera();
		camera->projectionDirty = 1;
		camera->fieldOfViewY = degrees * 0.017453292f;
		orbit.dirty = 1;
	}

	void CameraControl::Reset()
	{
		if (!constraints) return;
		transitionFlags = 0;
		movementFlags = 0;
		float low = minimumElevation * g_GfxDegreesToRadians;
		if (low >= -1.5620697736740112f) orbit.minimumElevation = low;
		float high = maximumElevation * g_GfxDegreesToRadians;
		if (high <= 1.5620697736740112f) orbit.maximumElevation = high;
		float lowerDistance = minimumDistance;
		if (lowerDistance >= 0.0f) orbit.minimumDistance = lowerDistance;
		float upperDistance = maximumDistance;
		if (upperDistance > orbit.minimumDistance) orbit.maximumDistance = upperDistance;
		float distance;
		if (minimumDistance != maximumDistance)
			distance = (maximumDistance - minimumDistance) * 0.5f + minimumDistance;
		else distance = minimumDistance;
		orbit.SetOrbit(0.0f, minimumElevation * g_GfxDegreesToRadians, distance);
		orbit.target = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		orbit.dirty = 1;
		float fov;
		if (minimumFieldOfView != maximumFieldOfView)
			fov = (maximumFieldOfView - minimumFieldOfView) * 0.5f + minimumFieldOfView;
		else fov = minimumFieldOfView;
		SetFieldOfView(fov);
	}

	void CameraControl::Update()
	{
		DWORD now = F3::GetElapsedTimeMS();
		DWORD elapsed = now - lastUpdateTime;
		if (elapsed < 33) return;
		lastUpdateTime = now;
		if (transitionFlags & 1)
		{
			currentAzimuth += azimuthSpeed * 0.001f * elapsed * azimuthDirection;
			if (azimuthDirection < 0.0f)
			{
				if (currentAzimuth <= targetAzimuth)
				{
					transitionFlags &= ~WORD(1); currentAzimuth = targetAzimuth;
				}
			}
			else if (currentAzimuth >= targetAzimuth)
			{
				transitionFlags &= ~WORD(1); currentAzimuth = targetAzimuth;
			}
			orbit.SetOrbit(currentAzimuth * g_GfxDegreesToRadians, orbit.GetElevation(), orbit.GetDistance());
		}
		if (transitionFlags & 2)
		{
			float elevation = elevationSpeed * 0.001f * elapsed * elevationDirection
				+ orbit.GetElevation() * 57.295776f;
			if (elevationDirection < 0.0f)
			{
				if (elevation <= targetElevation)
				{
					elevation = targetElevation; transitionFlags &= ~WORD(2);
				}
			}
			else if (elevation >= targetElevation)
			{
				elevation = targetElevation; transitionFlags &= ~WORD(2);
			}
			orbit.SetOrbit(orbit.GetAzimuth(), elevation * g_GfxDegreesToRadians, orbit.GetDistance());
		}
		if (transitionFlags & 4)
		{
			float distance = distanceSpeed * 0.001f * elapsed * distanceDirection + orbit.GetDistance();
			if (distanceDirection < 0.0f)
			{
				if (distance <= targetDistance)
				{
					distance = targetDistance; transitionFlags &= ~WORD(4);
				}
			}
			else if (distance >= targetDistance)
			{
				distance = targetDistance; transitionFlags &= ~WORD(4);
			}
			orbit.SetOrbit(orbit.GetAzimuth(), orbit.GetElevation(), distance);
		}
		if (transitionFlags & 8) transitionFlags &= ~WORD(8);
		if (movementFlags & 1)
			MoveTarget(-(GetFieldOfView() / maximumFieldOfView * (scrollSpeed * 0.001f) * elapsed), 0.0f, 0.0f);
		if (movementFlags & 2)
			MoveTarget(GetFieldOfView() / maximumFieldOfView * (scrollSpeed * 0.001f) * elapsed, 0.0f, 0.0f);
		if (movementFlags & 4)
			MoveTarget(0.0f, 0.0f, GetFieldOfView() / maximumFieldOfView * (scrollSpeed * 0.001f) * elapsed);
		if (movementFlags & 8)
			MoveTarget(0.0f, 0.0f, -(GetFieldOfView() / maximumFieldOfView * (scrollSpeed * 0.001f) * elapsed));
		if (movementFlags & 0x10)
		{
			float fov = GetFieldOfView() - fieldOfViewSpeed * 0.001f * elapsed;
			if (fov < minimumFieldOfView) fov = minimumFieldOfView;
			SetFieldOfView(fov);
		}
		if (movementFlags & 0x20)
		{
			float fov = fieldOfViewSpeed * 0.001f * elapsed + GetFieldOfView();
			if (fov > maximumFieldOfView) fov = maximumFieldOfView;
			SetFieldOfView(fov);
		}
		if (movementFlags & 0x40)
			orbit.Rotate(azimuthSpeed * 0.001f * elapsed * g_GfxDegreesToRadians, 0, 0);
		if (movementFlags & 0x80)
			orbit.Rotate(-(azimuthSpeed * 0.001f * elapsed) * g_GfxDegreesToRadians, 0, 0);
		if (movementFlags & 0x100)
			orbit.Rotate(0, elevationSpeed * 0.001f * elapsed * g_GfxDegreesToRadians, 0);
		if (movementFlags & 0x200)
			orbit.Rotate(0, -(elevationSpeed * 0.001f * elapsed) * g_GfxDegreesToRadians, 0);
	}

	void InitializeMainmenuCamera(CameraControl& control)
	{
		float x = 212.60001f, y = 0.0f, z = 329.20001f;
		float azimuth = 322.79999f, elevation = 24.200001f, distance = 75.0f;
		float fov = 45.0f, nearPlane = 10.0f, farPlane = 200.0f;
		const Configuration::Settings& settings = *F3::GetEngineSettings();
		Configuration::Settings::const_iterator section = settings.find("Mainmenu");
		if (section != settings.end())
		{
			const Configuration::Values& values = section->second;
			ReadFloat(values, "Target X", x); ReadFloat(values, "Target Y", y); ReadFloat(values, "Target Z", z);
			ReadFloat(values, "Azimuth", azimuth); ReadFloat(values, "Elevation", elevation);
			ReadFloat(values, "Distance", distance); ReadFloat(values, "FOV", fov);
			ReadFloat(values, "zNear", nearPlane); ReadFloat(values, "zFar", farPlane);
		}
		control.SetConstraints(0);
		control.SetAzimuth(azimuth);
		control.SetElevation(elevation);
		control.SetDistance(distance);
		control.SetFieldOfView(fov);
		control.SetTarget(D3DXVECTOR3(x, y, z));
		control.SetProjection(nearPlane, farPlane);
		control.client->child0x74->SetFogOfWarEnabled(0);
		g_GfxSystem->SetFlag2194(0);
	}
}
