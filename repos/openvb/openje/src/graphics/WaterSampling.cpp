#include "WaterSampling.hpp"
#include "ModelMesh.hpp"
#include "Water.hpp"

namespace
{
	BYTE waterSampling = 1; //Original6FC0BB data byte.
	JE::WaterSampleOwners waterOwners; //Actual static name index7099F4.
	BYTE removingWater = 0; //7099F1 reentrancy guard, not reset by owner destruction.
	BYTE waterLightmapSampling = 0; //7099F0.
}

namespace JE
{
	WaterSampleOwners& GetWaterSampleOwners() { return waterOwners; }
	BYTE GetWaterLightmapSampling() { return waterLightmapSampling; }
	Gfx_Water* FindWaterSampleOwner(const std::string& name)
	{
		WaterSampleOwners::iterator found=waterOwners.find(name);
		if (found!=waterOwners.end()) return static_cast<Gfx_Water*>(found->second);
		return NULL;
	}
	void RegisterWaterSampleOwner(Gfx_Water* owner)
	{
		const std::string& name=owner->registryName;
		WaterSampleOwners::iterator found=waterOwners.find(name);
		if (found==waterOwners.end()) waterOwners[name]=owner;
	}
	void RemoveWaterSampleOwners(std::string name)
	{
		if (removingWater) return;
		removingWater=1;
		WaterSampleOwners::iterator found=waterOwners.find(name);
		while (found!=waterOwners.end())
		{
			delete found->second; //Owner remains published while its destructor runs.
			found=waterOwners.erase(found);
			if (found==waterOwners.end() || waterOwners.empty()) break;
			found=waterOwners.lower_bound(name);
			found=(found!=waterOwners.end() &&
				name.compare(0,name.size(),found->first.c_str(),found->first.size())>=0)
				? found : waterOwners.end();
		}
		removingWater=0;
	}
	BYTE SetWaterSampling(BYTE enabled) { waterSampling = enabled; return enabled; }
	void AdvanceWaterSampling(DWORD milliseconds)
	{
		if (waterSampling)
		{
			WaterSampleOwners::iterator entry = waterOwners.begin();
			if (entry != waterOwners.end())
			{
				const float elapsed = static_cast<float>(milliseconds) * 0.001f;
				do
				{
					entry->second->Update(elapsed);
					++entry;
				} while (entry != waterOwners.end());
			}
		}
	}
}
