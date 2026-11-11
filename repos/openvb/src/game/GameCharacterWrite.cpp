#include "GamePlayer.hpp"
#include "GameInventory.hpp"
#include "entity/ChunkWriter.hpp"

namespace F3
{
	BYTE GameEntity::WriteToFile(JE::Resources::ResourceFileHandle* stream) //463970.
	{
		BYTE result = JE::Entity::WriteToFile(stream);
		if (!result) return result;
		DWORD type = 0x544E4547;
		DWORD version = 3;
		DWORD size = 0;
		const DWORD start = JE::Resources::g_pResourceSystem->Tell(stream);
		if (!JE::Resources::g_pResourceSystem->Write(&type, 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&version, 4, stream)) return 0;
		const DWORD sizeOffset = JE::Resources::g_pResourceSystem->Tell(stream);
		if (!JE::Resources::g_pResourceSystem->Write(&size, 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&stringRefs[0], 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&stringRefs[1], 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&stringRefs[2], 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&stringRefs[3], 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&stringRefs[4], 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&stringRefs[5], 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->Write(&maximumHealth, 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&currentHealth, 4, stream)) return 0;
		const DWORD end = JE::Resources::g_pResourceSystem->Tell(stream);
		size = end - start;
		JE::Resources::g_pResourceSystem->Seek(stream, sizeOffset, 0);
		if (!JE::Resources::g_pResourceSystem->Write(&size, 4, stream)) return 0;
		JE::Resources::g_pResourceSystem->Seek(stream, end, 0);
		return 1;
	}

	static BYTE WriteAttackMode(const AttackMode& mode,
		JE::Resources::ResourceFileHandle* stream)
	{
		JE::Common::WrittenChunk chunk;
		if (!JE::Common::BeginResourceChunk(stream, 0x4D415747, 5, chunk)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.animation, 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.damageType, 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.shots, 4, stream)) return 0;
		for (DWORD i = 0; i < 3; ++i)
			if (!JE::Resources::g_pResourceSystem->Write(mode.values18 + i, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->Write(&mode.range, 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(mode.values28, 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(mode.values28 + 1, 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.minimumDamage, 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.maximumDamage, 4, stream)) return 0;
		BYTE flag = mode.flag38 != 0;
		if (!JE::Resources::g_pResourceSystem->Write(&flag, 1, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.value0C, 4, stream)) return 0;
		flag = mode.calledShot != 0;
		if (!JE::Resources::g_pResourceSystem->Write(&flag, 1, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.actionPoints, 4, stream)) return 0;
		flag = mode.consumed != 0;
		if (!JE::Resources::g_pResourceSystem->Write(&flag, 1, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.thrown, 1, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.pose, 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.nameRef, 4, stream)
			|| !JE::Common::WriteString(mode.effectName, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.flag64, 1, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&mode.flag65, 1, stream)) return 0;
		return JE::Common::FinishResourceChunk(stream, chunk);
	}

	BYTE GameCreature::WriteToFile(JE::Resources::ResourceFileHandle* stream) //40BB80.
	{
		if (!GameEntity::WriteToFile(stream)) return 0;
		JE::Common::WrittenChunk chunk;
		if (!JE::Common::BeginResourceChunk(stream, 0x45524347, 5, chunk)) return 0;
		for (DWORD i = 0; i < 7; ++i)
			if (!JE::Resources::g_pResourceSystem->Write(special + i, 4, stream)) return 0;
		for (DWORD i = 0; i < 7; ++i)
			if (!JE::Resources::g_pResourceSystem->Write(baseValues + i, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->Write(&valueDC, 4, stream)) return 0;
		DWORD count = 13;
		if (!JE::Resources::g_pResourceSystem->Write(&count, 4, stream)) return 0;
		for (DWORD i = 0; i < 13; ++i)
			if (!JE::Resources::g_pResourceSystem->Write(&i, 4, stream)
				|| !JE::Resources::g_pResourceSystem->Write(skills + i, 4, stream)) return 0;
		count = 82;
		if (!JE::Resources::g_pResourceSystem->Write(&count, 4, stream)) return 0;
		for (DWORD i = 0; i < 82; ++i)
			if (!JE::Resources::g_pResourceSystem->Write(&i, 4, stream)
				|| !JE::Resources::g_pResourceSystem->Write(traits + i, 4, stream)) return 0;
		count = DWORD(selectedTraits.size());
		if (!JE::Resources::g_pResourceSystem->Write(&count, 4, stream)) return 0;
		for (std::list<DWORD>::iterator i = selectedTraits.begin(); i != selectedTraits.end(); ++i)
		{
			DWORD selectedTrait = *i;
			if (!JE::Resources::g_pResourceSystem->Write(&selectedTrait, 4, stream)) return 0;
		}
		count = DWORD(tagSkills.size());
		if (!JE::Resources::g_pResourceSystem->Write(&count, 4, stream)) return 0;
		for (std::list<DWORD>::iterator i = tagSkills.begin(); i != tagSkills.end(); ++i)
		{
			DWORD taggedSkill = *i;
			if (!JE::Resources::g_pResourceSystem->Write(&taggedSkill, 4, stream)) return 0;
		}
		if (!JE::Resources::g_pResourceSystem->Write(&value274, 4, stream)
			|| !JE::Common::WriteString(portrait, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&value294, 4, stream)
			|| !JE::Common::WriteString(value298, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&value2B4, 4, stream)
			|| !JE::Resources::g_pResourceSystem->Write(&materialMaskMode, 1, stream)) return 0;
		for (DWORD i = 0; i < 12; ++i)
			if (!JE::Common::WriteString(materialNames[i], stream)) return 0;
		for (DWORD i = 1; i < 13; ++i)
			if (!JE::Common::WriteString(sockets[i].model, stream)
				|| !JE::Common::WriteString(sockets[i].texture, stream)) return 0;
		for (DWORD i = 0; i < 12; ++i)
			if (!JE::Resources::g_pResourceSystem->Write(socketFlags + i, 1, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->Write(&attackModeCount, 4, stream)) return 0;
		for (DWORD i = 0; i < 8; ++i)
			if (!JE::Resources::g_pResourceSystem->Write(uncalledChances + i, 4, stream)) return 0;
		for (DWORD i = 0; i < 6; ++i)
			if (!JE::Resources::g_pResourceSystem->Write(baseResistances + i, 4, stream)) return 0;
		for (DWORD i = 0; i < 6; ++i)
			if (!JE::Resources::g_pResourceSystem->Write(baseThresholds + i, 4, stream)) return 0;
		count = 0;
		JE::Inventory* savedInventory = inventory;
		std::list<JE::Entity*> equipped;
		if (savedInventory)
		{
			GameInventory* items = dynamic_cast<GameInventory*>(savedInventory);
			if (items)
			{
				std::list<DWORD> slots;
				for (std::list<JE::Entity*>::iterator i = items->items.begin(); i != items->items.end(); ++i)
				{
					JE::Entity* item = *i;
					if (items->GetEquippedSlots(item, slots)) { ++count; equipped.push_back(item); }
				}
			}
		}
		if (!JE::Resources::g_pResourceSystem->Write(&count, 4, stream)) return 0;
		for (std::list<JE::Entity*>::iterator i = equipped.begin(); i != equipped.end(); ++i)
		{
			std::string name((*i)->resourceName);
			if (!JE::Common::WriteString(name, stream)) return 0;
		}
		if (!JE::Resources::g_pResourceSystem->Write(&valueD8, 4, stream)) return 0;
		DWORD index = 0;
		for (std::list<AttackMode>::iterator i = attackModes.begin();
			i != attackModes.end() && index < attackModeCount; ++i, ++index)
		{
			AttackMode mode(*i);
			if (!WriteAttackMode(mode, stream)) return 0;
		}
		return JE::Common::FinishResourceChunk(stream, chunk);
	}

	BYTE GamePlayer::WriteToFile(JE::Resources::ResourceFileHandle* stream) //411690.
	{
		if (!GameCreature::WriteToFile(stream)) return 0;
		JE::Common::WrittenChunk chunk;
		if (!JE::Common::BeginResourceChunk(stream, 0x52484347, 0, chunk)
			|| !JE::Common::WriteString(characterName, stream)) return 0;
		return JE::Common::FinishResourceChunk(stream, chunk);
	}
}
