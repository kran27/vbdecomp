#include "Entity.hpp"
#include "Inventory.hpp"
#include "network/Packet.hpp"
#include "model/CommonException.hpp"
#include <cstring>

namespace JE
{
	namespace
	{
		void WriteFloatWord(Net::Packet& packet, const float& value)
		{
			Net::UInt32 bits;
			std::memcpy(&bits, &value, sizeof(bits));
			packet.WriteDword(bits);
		}
	}

	void Entity::PackState(Net::Packet& packet) //54A610.
	{
		const char* display = displayName.c_str();
		packet.Write(display, std::strlen(display) + 1);
		packet.WriteSignedDword(static_cast<Net::Int32>(state)); //45F0B0, preserve raw32-bit transport.
		packet.WriteDword(visibilityOwnerFlag);
		packet.WriteDword(mapHandle);
		WriteFloatWord(packet, position.x);
		WriteFloatWord(packet, position.y);
		WriteFloatWord(packet, position.z);
		WriteFloatWord(packet, rotation.x);
		WriteFloatWord(packet, rotation.y);
		WriteFloatWord(packet, rotation.z);
		const char* modelNameText = modelName.c_str();
		packet.Write(modelNameText, std::strlen(modelNameText) + 1);
		const char* textureText = textureOverride.c_str();
		packet.Write(textureText, std::strlen(textureText) + 1);
		const char* inventoryText = inventoryTexture.c_str();
		packet.Write(inventoryText, std::strlen(inventoryText) + 1);
		packet.WriteDword(currentAnimationGroup);
		packet.WriteDword(currentAnimationIndex);
		packet.WriteDword(animationStartTime);
		packet.WriteDword(animationEndTime);
		packet.WriteByte(flag296);
		packet.WriteByte(frozenAnimation);
		packet.WriteDword(frozenAnimationTime);
		packet.WriteDword(value29C);
		if (inventory)
		{
			packet.WriteByte(1);
			inventory->PackState(packet); //Reload the current owner after the presence write.
		}
		else packet.WriteByte(0);
	}

	void Entity::UnpackState(DWORD id, Net::Packet& packet) //54E690.
	{
		if (state)
			throw ::Common::Exception("UnpackState() method may only be called on invalid entities.");
		entityID=id;
		char text[256];
		packet.ReadCString(text); displayName=text;
		Net::UInt32 word;
		packet.ReadDword(word); state=word;
		packet.ReadDword(visibilityOwnerFlag);
		packet.ReadDword(mapHandle);
		D3DXVECTOR3 nextPosition(0,0,0);
		packet.ReadDword(word); std::memcpy(&nextPosition.x,&word,sizeof(word));
		packet.ReadDword(word); std::memcpy(&nextPosition.y,&word,sizeof(word));
		packet.ReadDword(word); std::memcpy(&nextPosition.z,&word,sizeof(word));
		D3DXVECTOR3 nextRotation(0,0,0);
		packet.ReadDword(word); std::memcpy(&nextRotation.x,&word,sizeof(word));
		packet.ReadDword(word); std::memcpy(&nextRotation.y,&word,sizeof(word));
		packet.ReadDword(word); std::memcpy(&nextRotation.z,&word,sizeof(word));
		packet.ReadCString(text); modelName=text;
		packet.ReadCString(text); textureOverride=text;
		packet.ReadCString(text); inventoryTexture=text;
		packet.ReadDword(currentAnimationGroup);
		packet.ReadDword(currentAnimationIndex);
		packet.ReadDword(animationStartTime);
		packet.ReadDword(animationEndTime);
		packet.ReadBool(flag296);
		packet.ReadBool(frozenAnimation);
		packet.ReadDword(frozenAnimationTime);
		packet.ReadDword(value29C);
		Net::UInt8 hasInventory;
		packet.ReadByte(hasInventory);
		if (hasInventory==1)
		{
			if (!inventory) inventory=CreateInventory();
			inventory->UnpackState(packet);
		}
		SetModel(modelName.c_str(),0);
		SetPosition(nextPosition);
		SetRotation(nextRotation);
	}
}
