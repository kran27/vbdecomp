#include "Map.hpp"
#include "MapEffect.hpp"
#include "World.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "resources/ResourceSystem.hpp"
#include <cstring>
#include <cctype>
#include <stdexcept>

namespace JE
{
	namespace
	{
		DWORD FourCC(const char* name)
		{ return BYTE(name[0]) | (DWORD(BYTE(name[1]))<<8) | (DWORD(BYTE(name[2]))<<16) | (DWORD(BYTE(name[3]))<<24); }
	}
	BYTE Map::ReadResource(const char* name)
	{
		ClearNavigation();
		ClearWaypoints();
		ClearSounds();
		ClearMapEffects();
		Resources::ResourceMemoryStream* stream=Resources::g_pResourceSystem->OpenMemoryStream(name,0);
		if(!stream) return 0;
		BYTE success=1;
		Common::Chunk chunk;
		if(chunk.Read(stream))
		{
			do
			{
				if(!ReadChunk(chunk,stream))
				{ LogDebugString("Error reading chunk 0x%0x from map file %s\n",chunk.type,name); success=0; break; }
				chunk.SkipToEnd();
			}
			while(chunk.Read(stream));
		}
		Resources::g_pResourceSystem->CloseMemoryStream(stream);
		return success; // Original empty-resource/no-first-chunk quirk.
	}
	BYTE Map::ReadChunk(Common::Chunk& chunk,Resources::ResourceMemoryStream* stream)
	{
		const DWORD type = chunk.type;
		BYTE result;
		const char* error;
		if (type == FourCC("EMAP"))
		{
			result = ReadMap(chunk,stream);
			error = "Error reading header from map file.\n";
		}
		else if (type == FourCC("EME2"))
		{
			result = ReadEntity(chunk,stream);
			error = "Error reading entity from map file.\n";
		}
		else if (type == FourCC("ECAM"))
		{
			result = ReadCamera(chunk,stream);
			error = "Error reading camera constraints from map file.\n";
		}
		else if (type == FourCC("EMNP"))
		{
			result = ReadNavigation(chunk,stream);
			error = "Error reading nav point from map file.\n";
		}
		else if (type == FourCC("EMEF"))
		{
			result = ReadEffect(chunk,stream);
			error = "Error reading map effect from map file.\n";
		}
		else if (type == FourCC("EMEP"))
		{
			result = ReadEntryPoint(chunk,stream);
			error = "Error reading entry point from map file.\n";
		}
		else if (type == FourCC("EMTR"))
		{
			result = ReadTrigger(chunk,stream);
			error = "Error reading trigger from map file.\n";
		}
		else if (type == FourCC("EMEN"))
		{
			result = ReadLegacyEntity(chunk,stream);
			error = "Error reading entity from map file.\n";
		}
		else if (type == FourCC("2MWT"))
		{
			result = ReadWater(chunk,stream);
			error = "Error reading water from map file.\n";
		}
		else if (type == FourCC("EMFG"))
		{
			result = ReadHeightFog(chunk,stream);
			error = "Error reading fog from map file.\n";
		}
		else if (type == FourCC("EMNO"))
		{
			result = ReadNote(chunk,stream);
			error = "Error reading note from map file.\n";
		}
		else if (type == FourCC("EPTH"))
		{
			result = ReadWaypoints(chunk,stream);
			error = "Error reading way point from map file.\n";
		}
		else if (type == FourCC("EMSD"))
		{
			result = ReadSound(chunk,stream);
			error = "Error reading map sound from map file.\n";
		}
		else return 1; //Unknown chunks are skipped by the original dispatcher.
		if (!result)
		{
			LogDebugString(error);
			return 0;
		}
		return 1;
	}
	BYTE Map::ReadLegacyEntity(Common::Chunk& chunk,Resources::ResourceMemoryStream* stream) //573BF0.
	{
		LogDebugString("Warning: reading obsolete entity chunk from map file.\n");
		const DWORD version = chunk.version;
		Entity* entity;
		{
			std::string resource;
			if (!Common::ReadString(resource,stream))
			{
				return 0;
			}
			DWORD type = g_World->ClassifyEntity(resource,0);
			entity = g_World->NewEntity(type);
			entity->Create(resource.c_str(),0,0);
			D3DXVECTOR3 position(0,0,0);
			if (!Common::ReadMemory(&position.x,4,stream)
				|| !Common::ReadMemory(&position.y,4,stream)
				|| !Common::ReadMemory(&position.z,4,stream))
			{
				if (entity)
				{
					delete entity;
				}
				return 0;
			}
			entity->SetPosition(position);
			D3DXVECTOR3 angles(0,0,0);
			if (!Common::ReadMemory(&angles.x,4,stream)
				|| !Common::ReadMemory(&angles.y,4,stream)
				|| !Common::ReadMemory(&angles.z,4,stream))
			{
				if (entity)
				{
					delete entity;
				}
				return 0;
			}
			//647920 contains float bits42652EE0, including the original rounding.
			angles.x *= 57.295776f;
			angles.y *= 57.295776f;
			angles.z *= 57.295776f;
			entity->SetRotation(angles);
		} //The resource string dies before either versioned field is read.
		if (version)
		{
			std::string name;
			if (!Common::ReadString(name,stream))
			{
				if (entity)
				{
					delete entity;
				}
				return 0;
			}
			entity->SetDisplayName(name);
		}
		if (version >= 2)
		{
			BYTE shadow;
			if (!Common::ReadMemory(&shadow,1,stream))
			{
				if (entity)
				{
					delete entity;
				}
				return 0;
			}
			entity->SetShadow(shadow != 0);
		}
		entities.push_back(entity);
		return 1;
	}
	BYTE Map::ReadMap(Common::Chunk& chunk,Resources::ResourceMemoryStream* stream)
	{
		const DWORD version = chunk.version; //56C9D6 captures before the first read.
		if(!Resources::g_pResourceSystem->ReadMemory(&value28,4,stream)
			|| !Common::ReadString(octreeName,stream)
			|| !Common::ReadString(gridName,stream)
			|| !Common::ReadString(areaTextureName,stream)
			|| !Common::ReadString(scriptName,stream)) return 0;
		if(version>=1)
		{
			if(!Resources::g_pResourceSystem->ReadMemory(color+0,1,stream)) return 0;
			if(!Resources::g_pResourceSystem->ReadMemory(color+1,1,stream)) return 0;
			if(!Resources::g_pResourceSystem->ReadMemory(color+2,1,stream)) return 0;
			ambient[0]=color[0];
			ambient[1]=color[1];
			ambient[2]=color[2];
			ambient[3]=0x80;
		}
		BYTE value;
		if(version>=2) { if(!Resources::g_pResourceSystem->ReadMemory(&value,1,stream)) return 0; flag1D8=value!=0; }
		if(version>=3)
		{
			if(!Resources::g_pResourceSystem->ReadMemory(ambient+0,1,stream)) return 0;
			if(!Resources::g_pResourceSystem->ReadMemory(ambient+1,1,stream)) return 0;
			if(!Resources::g_pResourceSystem->ReadMemory(ambient+2,1,stream)) return 0;
			if(!Resources::g_pResourceSystem->ReadMemory(ambient+3,1,stream)) return 0;
		}
		if(version>=4) { if(!Resources::g_pResourceSystem->ReadMemory(&value,1,stream)) return 0; flag1D9=value!=0; }
		if(version>=5)
		{
			if(!Resources::g_pResourceSystem->ReadMemory(&value,1,stream)) return 0;
			flag1B1=value!=0;
			BYTE red,green,blue;
			if(!Resources::g_pResourceSystem->ReadMemory(&red,1,stream) || !Resources::g_pResourceSystem->ReadMemory(&green,1,stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&blue,1,stream)) return 0;
			fogColor=((((DWORD(-1) << 8) | red) << 8) | green) << 8 | blue;
			if(!Resources::g_pResourceSystem->ReadMemory(&fogNear,4,stream) || !Resources::g_pResourceSystem->ReadMemory(&fogFar,4,stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&fogScale,4,stream)) return 0;
			fogFlagCopy=flag1B1; fogColorCopy=float(fogColor);
			fogNearCopy=fogNear; fogFarCopy=fogFar; fogScaleCopy=fogScale;
		}
		return 1;
	}
	BYTE Map::ReadCamera(Common::Chunk&,Resources::ResourceMemoryStream* stream)
	{
		if(Resources::g_pResourceSystem->ReadMemory(&cameraLow.x,4,stream)
			&& Resources::g_pResourceSystem->ReadMemory(&cameraLow.z,4,stream)
			&& Resources::g_pResourceSystem->ReadMemory(&cameraHigh.x,4,stream)
			&& Resources::g_pResourceSystem->ReadMemory(&cameraHigh.z,4,stream))
		{
			cameraConstraints=1;
			return 1;
		}
		return 0;
	}
	BYTE Map::ReadEffect(Common::Chunk&,Resources::ResourceMemoryStream* stream)
	{
		std::string name;
		D3DXVECTOR3 position(0,0,0),angles(0,0,0);
		if(!Common::ReadString(name,stream) || !Common::ReadMemory(&position.x,4,stream)
			|| !Common::ReadMemory(&position.y,4,stream) || !Common::ReadMemory(&position.z,4,stream)
			|| !Common::ReadMemory(&angles.x,4,stream) || !Common::ReadMemory(&angles.y,4,stream)
			|| !Common::ReadMemory(&angles.z,4,stream)) return 0;
		BYTE count,enabled;
		if(!Common::ReadMemory(&count,1,stream)) return 0;
		std::list<std::string> names;
		for(DWORD i=0;i<count;++i)
		{
			std::string resource;
			if(!Common::ReadString(resource,stream)) return 0;
			names.push_back(resource);
		}
		if(!Common::ReadMemory(&enabled,1,stream)) return 0;
		MapEffect* effect=new MapEffect(this);
		effect->name=name; effect->position=position; effect->angles=angles;
		effect->enabled=enabled!=0;
		effect->resources.assign(names.begin(),names.end());
		for(size_t i=0;i<name.size();++i) name[i]=char(toupper((unsigned char)name[i]));
		if(mapEffects.find(name)!=mapEffects.end())
		{
			delete effect;
			throw std::runtime_error(std::string("Duplicate instance name in Map::ReadMapEffect(): ")+name+".");
		}
		mapEffects[name]=effect;
		return 1;
	}
}
