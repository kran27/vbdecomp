#include "PacketData.hpp"
#include "SkinMeshPacket.hpp"
#include "Light.hpp"
#include "SystemInterface.hpp"
#include <algorithm>

namespace
{
	struct LightOrder
	{
		bool operator()(const Gfx_Light* left, const Gfx_Light* right) const
		{
			// Original merge moves equal signed priorities before the held node.
			return static_cast<LONG>(left->value44) >= static_cast<LONG>(right->value44);
		}
	};
}

BYTE Gfx_BasePacketData::AddLight(Gfx_Light* light) //4B7660.
{
	BYTE result = 0;
	if (lights.size() < g_GfxSystem->MaximumActiveLights())
	{
		lights.push_back(light);
		result = 1;
	}
	return result;
}
void Gfx_BasePacketData::RemoveLight(Gfx_Light* light) //4B73A0.
{
	for (Gfx_LightList::iterator found = lights.begin(); found != lights.end(); found++)
	{
		if (*found == light)
		{
			lights.erase(found);
			break;
		}
	}
	for (std::map<DWORD,Gfx_LightList>::iterator i = primitiveLights.begin(); i != primitiveLights.end(); i++)
	{
		for (Gfx_LightList::iterator found = i->second.begin(); found != i->second.end(); found++)
		{
			if (*found == light)
			{
				i->second.erase(found);
				break;
			}
		}
	}
}
void Gfx_BasePacketData::FinishLights() //4B7E70, actual STL sort reused.
{
	LightOrder order;
	lights.sort(order);
	for (std::map<DWORD,Gfx_LightList>::iterator i = primitiveLights.begin(); i != primitiveLights.end(); ++i)
		i->second.sort(order);
}

BYTE Gfx_SkinMeshPkt::AddLight(Gfx_Light* light)
{
	BYTE result = 0;
	if (lights.size() < g_GfxSystem->MaximumActiveLights())
	{
		lights.push_back(light);
		result = 1;
	}
	return result;
}

void Gfx_SkinMeshPkt::RemoveLight(Gfx_Light* light)
{
	for (Gfx_LightList::iterator found = lights.begin(); found != lights.end(); found++)
	{
		if (*found == light)
		{
			lights.erase(found);
			break;
		}
	}
}

void Gfx_SkinMeshPkt::FinishLights()
{
}
