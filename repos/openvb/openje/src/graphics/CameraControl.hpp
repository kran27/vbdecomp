#pragma once

#include "CameraController.hpp"
#include "configuration/ConfigurationTypes.hpp"

namespace JE
{
	class Client;

	class CameraControl
	{
	public:
		explicit CameraControl(Client* client); //586F70; embeds, does not derive from Orbit.
		~CameraControl(); //586570; ordinary destructor, borrows Client/cameras.
		virtual void LoadConfiguration(); //587030, actual first virtual slot.
		virtual void OnCleanup(); //Shared404B70 no-op; name is a source hypothesis.
		void GetTarget(D3DXVECTOR3& output) const; //585E20.
		void GetScrollBorders(LONG& width, LONG& height); //586230, actual unsigned dimension arithmetic.
		void SetAzimuth(float degrees); //585E60.
		void SetTargetAzimuth(float degrees); //585EA0, shortest transition with ceil equality.
		void SetElevation(float degrees); //585FD0.
		void SetTargetElevation(float degrees); //586010, configured limits.
		void SetDistance(float distance); //586110.
		void SetTargetDistance(float distance); //586140, bounded distance transition.
		void ToggleTopDown(BYTE transition); //586290.
		void Top(BYTE transition); //5862F0.
		void Down(BYTE transition); //586310.
		void SetTarget(D3DXVECTOR3 target); //586760, actual12B value/RET12.
		void MoveTarget(float x, float y, float z); //586410, camera-relative horizontal basis.
		void SetConstraints(BYTE enabled); //586590, raw flag.
		void SetProjection(float nearPlane, float farPlane); //586710.
		void SetFieldOfView(float degrees); //5867C0.
		float GetFieldOfView() const; //586550, actual x87 degree result.
		void Reset(); //586830; only acts when constraints are enabled.
		void Update(); //5869C0; absolute clock and33ms minimum step.

		Client* client; //Original+04, borrowed.
		Gfx_CamCtrl_Orbit orbit; //Original+08, actual embedded owner.
		WORD transitionFlags, movementFlags; //Original+C0/+C2.
		DWORD lastUpdateTime; //+C4.
		float azimuthSpeed, targetAzimuth, azimuthDirection, currentAzimuth; //+C8..D4, degrees.
		float elevationSpeed, minimumElevation, maximumElevation, targetElevation, elevationDirection; //+D8..E8.
		float distanceSpeed, minimumDistance, maximumDistance, targetDistance, distanceDirection; //+EC..FC.
		float fieldOfViewSpeed, minimumFieldOfView, maximumFieldOfView; //+100..108.
		float scrollBorder, scrollSpeed; //+10C/+110.
		BYTE constraints; //+114; padding stays unwritten.
		float savedNearPlane, savedFarPlane; //+118/+11C.

	private:
		void ClampTarget(D3DXVECTOR3& target); //586330.
		CameraControl(const CameraControl&);
		CameraControl& operator=(const CameraControl&);
	};

	// Actual460D60 policy with explicit borrowed CameraControl/Client binding;
	// the surrounding GameClient startup is not supplied by this factor.
	void InitializeMainmenuCamera(CameraControl& control);
}
