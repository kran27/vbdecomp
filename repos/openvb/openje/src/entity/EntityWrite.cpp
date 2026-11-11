#include "Entity.hpp"
#include "ChunkWriter.hpp"
#include "Inventory.hpp"
#include "graphics/VfxVeg.hpp"

namespace JE
{
	BYTE Entity::WriteToFile(Resources::ResourceFileHandle* stream) //54F6A0.
	{
		Common::WrittenChunk entityChunk, overridesChunk;
		//The original writes seven local operands without defining their storage:
		//EEN2 three bytes, EEOV three bytes and one DWORD. Preserve all ten bytes
		//as unwritten object storage, rather than inventing reserved values.
		BYTE unwrittenEEN2[3], unwrittenEEOV[3], unwrittenEEOVWord[4];
		if (!Common::BeginResourceChunk(stream, 0x324E4545, 0, entityChunk)
			|| !Common::WriteString(modelName, stream)
			|| !Common::WriteString(inventoryTexture, stream)
			|| !Common::WriteString(activeTexture, stream)) return 0;
		BYTE serializedFlag = flag296;
		if (!Resources::g_pResourceSystem->Write(&serializedFlag, 1, stream)) return 0;
		serializedFlag = selectable;
		if (!Resources::g_pResourceSystem->Write(&serializedFlag, 1, stream)) return 0;
		for (DWORD i = 0; i < 3; ++i)
			if (!Resources::g_pResourceSystem->Write(unwrittenEEN2 + i, 1, stream)) return 0;
		if (!Common::BeginResourceChunk(stream, 0x564F4545, 2, overridesChunk)
			|| !Common::WriteString(displayName, stream)) return 0;
		for (DWORD i = 0; i < 3; ++i)
			if (!Resources::g_pResourceSystem->Write(unwrittenEEOV + i, 1, stream)) return 0;
		if (!Resources::g_pResourceSystem->Write(&value17C, 4, stream)
			|| !Resources::g_pResourceSystem->Write(&value36C, 4, stream)
			|| !Common::WriteString(roleName, stream)
			|| !Common::WriteString(scriptName, stream)) return 0;
		DWORD variableCount = DWORD(variables.size());
		if (!Resources::g_pResourceSystem->Write(&variableCount, 4, stream)) return 0;
		for (std::map<std::string,cell>::iterator i = variables.begin(); i != variables.end(); ++i)
		{
			std::string name(i->first);
			DWORD scriptVariableValue = i->second;
			if (!Common::WriteString(name, stream)
				|| !Resources::g_pResourceSystem->Write(&scriptVariableValue, 4, stream)) return 0;
		}
		BYTE serializedShadow = shadow;
		if (!Resources::g_pResourceSystem->Write(&serializedShadow, 1, stream)
			|| !Resources::g_pResourceSystem->Write(unwrittenEEOVWord, 4, stream)
			|| !Common::WriteString(textureOverride, stream)) return 0;
		BYTE effectCount = BYTE(activeEffects.size());
		if (!Resources::g_pResourceSystem->Write(&effectCount, 1, stream)) return 0;
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = activeEffects.begin();
			i != activeEffects.end(); ++i)
		{
			std::string name;
			if (i->second.instance) name = i->second.instance->name; //Original node+20, not the map key.
			if (!Common::WriteString(name, stream)) return 0;
		}
		BYTE serializedEnabled = enabled;
		if (!Resources::g_pResourceSystem->Write(&serializedEnabled, 1, stream)) return 0;
		BYTE serializedFlag17A = flag17A;
		if (!Resources::g_pResourceSystem->Write(&serializedFlag17A, 1, stream)) return 0;
		Inventory* savedInventory = inventory;
		DWORD itemCount = 0;
		std::list<Entity*> unequipped;
		if (savedInventory)
		{
			std::list<DWORD> slots;
			for (std::list<Entity*>::iterator i = savedInventory->items.begin();
				i != savedInventory->items.end(); ++i)
			{
				Entity* item = *i;
				if (!savedInventory->GetEquippedSlots(item, slots))
				{
					++itemCount;
					unequipped.push_back(item);
				}
			}
		}
		if (!Resources::g_pResourceSystem->Write(&itemCount, 4, stream)) return 0;
		for (std::list<Entity*>::iterator i = unequipped.begin(); i != unequipped.end(); ++i)
		{
			std::string name((*i)->resourceName);
			if (!Common::WriteString(name, stream)) return 0;
		}
		DWORD end = Resources::g_pResourceSystem->Tell(stream);
		if (!Common::PatchResourceChunk(stream, overridesChunk, end)
			|| !Common::PatchResourceChunk(stream, entityChunk, end)) return 0;
		Resources::g_pResourceSystem->Seek(stream, end, 0);
		return 1;
	}
}
