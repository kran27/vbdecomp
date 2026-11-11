#pragma once

#include "GameEntity.hpp"
#include <list>

namespace F3
{
	class GameItem : public GameEntity
	{
		friend JE::Entity* CreateGameEntity(DWORD type);
	public:
		virtual void CopyFrom(JE::Entity* source); //411330, serialized item fields before GameEntity.
		virtual DWORD GetContextAction(JE::Entity* actor); //410440/Weapon4146A0->4100B0,13.
		virtual void GetContextActions(JE::Entity* actor, std::list<JE::ContextMenuEntry>& entries); //411250.
		virtual BYTE ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); // 0x411260/411320.
		BYTE ReadItem(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
			BYTE skipDefaultOverride); //4111E0; third argument is passed but unused.
		BYTE ReadVersion0(JE::Resources::ResourceMemoryStream* stream); // 0x410B80.
		const std::string& GetSocketModel(DWORD slot) const; // 0x4100C0.
		LONG value160, value164;
		std::list<DWORD> values168;
		std::string socketModel1, socketTexture1;
		BYTE hideHair, hideBeard, hideMoustache, hideEyes, hidePonytail;
		std::string socketModel2, socketTexture2;
		BYTE socketFlag2;
		std::string socketModel5, socketTexture5;
		BYTE socketFlag5;
		std::string socketModel4, socketTexture4;
		BYTE socketFlag4;
		std::string socketModel6, socketTexture6;
		BYTE socketFlag6;
		std::string socketModel3, socketTexture3;
		BYTE socketFlag3;
		std::string socketModel7, socketTexture7;
		std::string socketModel10, socketTexture10;
		std::string socketModel8, socketTexture8;
		BYTE hideVanity;
		std::string socketModel9, socketTexture9;
		std::string socketModel11, socketTexture11;
		std::string socketModel12, socketTexture12;
		BYTE socketFlag12;
		std::string heldModel, heldTexture;
	protected:
		GameItem(); //410180; original interleaved string/flag ownership.
	};
}

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
#include <cstddef>
#define F3_ITEM_RELATIVE(name, original) \
	typedef char F3_Item_##name[(offsetof(F3::GameItem, name) - \
		offsetof(F3::GameItem, value160) == (original) - 0xA0) ? 1 : -1]
F3_ITEM_RELATIVE(value160, 0xA0);
F3_ITEM_RELATIVE(value164, 0xA4);
F3_ITEM_RELATIVE(values168, 0xA8);
F3_ITEM_RELATIVE(socketModel1, 0xB4);
F3_ITEM_RELATIVE(socketTexture1, 0xD0);
F3_ITEM_RELATIVE(hideHair, 0xEC);
F3_ITEM_RELATIVE(hideBeard, 0xED);
F3_ITEM_RELATIVE(hideMoustache, 0xEE);
F3_ITEM_RELATIVE(hideEyes, 0xEF);
F3_ITEM_RELATIVE(hidePonytail, 0xF0);
F3_ITEM_RELATIVE(socketModel2, 0xF4);
F3_ITEM_RELATIVE(socketTexture2, 0x110);
F3_ITEM_RELATIVE(socketFlag2, 0x12C);
F3_ITEM_RELATIVE(socketModel5, 0x130);
F3_ITEM_RELATIVE(socketTexture5, 0x14C);
F3_ITEM_RELATIVE(socketFlag5, 0x168);
F3_ITEM_RELATIVE(socketModel4, 0x16C);
F3_ITEM_RELATIVE(socketTexture4, 0x188);
F3_ITEM_RELATIVE(socketFlag4, 0x1A4);
F3_ITEM_RELATIVE(socketModel6, 0x1A8);
F3_ITEM_RELATIVE(socketTexture6, 0x1C4);
F3_ITEM_RELATIVE(socketFlag6, 0x1E0);
F3_ITEM_RELATIVE(socketModel3, 0x1E4);
F3_ITEM_RELATIVE(socketTexture3, 0x200);
F3_ITEM_RELATIVE(socketFlag3, 0x21C);
F3_ITEM_RELATIVE(socketModel7, 0x220);
F3_ITEM_RELATIVE(socketTexture7, 0x23C);
F3_ITEM_RELATIVE(socketModel10, 0x258);
F3_ITEM_RELATIVE(socketTexture10, 0x274);
F3_ITEM_RELATIVE(socketModel8, 0x290);
F3_ITEM_RELATIVE(socketTexture8, 0x2AC);
F3_ITEM_RELATIVE(hideVanity, 0x2C8);
F3_ITEM_RELATIVE(socketModel9, 0x2CC);
F3_ITEM_RELATIVE(socketTexture9, 0x2E8);
F3_ITEM_RELATIVE(socketModel11, 0x304);
F3_ITEM_RELATIVE(socketTexture11, 0x320);
F3_ITEM_RELATIVE(socketModel12, 0x33C);
F3_ITEM_RELATIVE(socketTexture12, 0x358);
F3_ITEM_RELATIVE(socketFlag12, 0x374);
F3_ITEM_RELATIVE(heldModel, 0x378);
F3_ITEM_RELATIVE(heldTexture, 0x394);
#undef F3_ITEM_RELATIVE
typedef char F3_ItemOwnExtent[(sizeof(F3::GameItem) - sizeof(F3::GameEntity) == 0x310) ? 1 : -1];
#endif
