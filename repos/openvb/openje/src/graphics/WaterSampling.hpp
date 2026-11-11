#pragma once

#include <windows.h>
#include <map>
#include <string>

class Gfx_NodeAttribute;
class Gfx_Water;

namespace JE
{
	// Original7099F4 name index. Actual Gfx_Water supplies the existing
	// Gfx_NodeAttribute Update(float) API; no water simulation is supplied here.
	typedef std::map<std::string, Gfx_NodeAttribute*> WaterSampleOwners;
	WaterSampleOwners& GetWaterSampleOwners();
	Gfx_Water* FindWaterSampleOwner(const std::string& name); //51DAF0.
	void RegisterWaterSampleOwner(Gfx_Water* owner); //51E8C0, retain an existing name.
	void RemoveWaterSampleOwners(std::string name); //51DE30, exact-key selection.
	BYTE GetWaterLightmapSampling(); //Original loader-zero byte7099F0.
	BYTE SetWaterSampling(BYTE enabled); //51CC50, original6FC0BB initially1.
	void AdvanceWaterSampling(DWORD milliseconds); //51DA40, original ignores incidental AL.
}
