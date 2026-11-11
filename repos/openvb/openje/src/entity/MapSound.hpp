#pragma once

#include "DynamicObject.hpp"
#include <string>

namespace JE
{
	class VfxSoundInstance; //Shared actual52E2E0 owner; populated audio backend remains unresolved.
	char UpperMapSoundByte(char value); //4A11E0, shared native byte-uppercase policy.
	class MapSound
	{
	public:
		MapSound(); //5B2AB0.
		virtual ~MapSound(); //5B2AF0.
		void Clear(); //5B29D0, releases the owned sound before null publication.
		void Initialize(); //5B2A00, always Clear then load a nonempty resource.
		void Start(); //5B2900, current looping flag and argument0.
		void Stop(LONG argument); //5B2920; does not change enabled.
		void SetEnabled(BYTE value); //5B2990, truth-value compare then raw BYTE store.
		void Update(DWORD elapsed); //5B2930, elapsed argument unused by the original.
		std::string name; //Original+4.
		D3DXVECTOR3 position; //+20.
		std::string resourceName; //+2C.
		BYTE looping, enabled; //+48/+49, constructor1.
		VfxSoundInstance* sound; //+4C, owning; constructorNULL.
	private:
		MapSound(const MapSound&);
		MapSound& operator=(const MapSound&);
	};
}
