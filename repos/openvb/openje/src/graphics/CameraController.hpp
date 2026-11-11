#pragma once

#include <d3dx8.h>
#include <map>

class Gfx_Camera;

// Actual three-slot CamCtrl interface. Offset comments describe the VC7 lane;
// host builds own normal STL objects and do not overlay original storage.
class Gfx_CamCtrl
{
public:
	Gfx_CamCtrl(); // 0x4C6740.
	virtual ~Gfx_CamCtrl() = 0; // Shared ordinary cleanup tail0x529FB0.
	virtual BYTE Update(); // +04 ->0x529EF0.
	virtual void ComputeTransform() = 0; // +08, pure in the base table.
	void ExtractBasis(); // 0x4C6200; unused original return is not exposed.
	void RemoveCamera(Gfx_Camera* camera); // 0x4264D0; borrows the camera.

	BYTE dirty; // +04; padding+05..07 is not initialized by game policy.
	std::map<Gfx_Camera*, BYTE> cameras; // +08; owns nodes, borrows keys.
	D3DXMATRIX transform; // +14, camera world transform.
	D3DXVECTOR3 right, up, look, position; // +54/+60/+6C/+78.

private:
	Gfx_CamCtrl(const Gfx_CamCtrl&);
	Gfx_CamCtrl& operator=(const Gfx_CamCtrl&);
};

class Gfx_CamCtrl_Orbit : public Gfx_CamCtrl
{
public:
	Gfx_CamCtrl_Orbit(); // 0x4C67C0; independently observed184-byte allocation.
	virtual ~Gfx_CamCtrl_Orbit() {} // 0x4C68A0, then shared base cleanup.
	virtual void ComputeTransform(); // +08 ->0x4C6430.
	void SetOrbit(float azimuth, float elevation, float distance); // 0x424670.
	void Rotate(float azimuth, float elevation, float distance); // 0x4C62B0, actual ECX/RET12.
	void TranslateTarget(float x, float y, float z); //0x4C62F0, actual ECX/RET12.
	//Float-returning inline reads preserve the original x87 value transport.
	float GetAzimuth() const { return azimuth; }
	float GetElevation() const { return elevation; }
	float GetDistance() const { return distance; }

	D3DXVECTOR3 upAxis; // +84; distinct from extracted basis up.
	D3DXVECTOR3 target; // +90.
	float azimuth, elevation, distance; // +9C/+A0/+A4, angles in radians.
	float minimumElevation, maximumElevation; // +A8/+AC.
	float minimumDistance, maximumDistance; // +B0/+B4.
};
