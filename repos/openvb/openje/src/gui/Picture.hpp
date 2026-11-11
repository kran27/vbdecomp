#pragma once

#include "Window.hpp"

namespace GUI
{
	// Actual Window-derived owner: VC7 sizeof3372, tables66DA34/66DA28.
	class Picture : public Window
	{
	public:
		explicit Picture(DWORD type); // 4AA8B0.
		virtual ~Picture(); // Ordinary4AAAA0.
		virtual BYTE ReadResource(JE::Resources::ResourceFileHandle* file, int version); // 4AB190.
		virtual BYTE ReadFile(FILE* file, int version); // 4AB160; base FILE path unresolved.
		virtual void ApplyProfile(const std::string& skin, const std::string& profile); // 4AA9B0.
		virtual void ApplySkin(const std::string& skin); // 4AB490.
		// Added slots+12C..13C. Argument spellings/unused returns remain H-TYPE/H-RETURN.
		virtual void SetTexture(DWORD handle, BYTE resize, LONG width, LONG height,
			float x, float y, float sourceWidth, float sourceHeight, DWORD fit, BYTE owned); //4AAB20.
		virtual void LoadTexture(const std::string& name, BYTE resize, float x, float y,
			float sourceWidth, float sourceHeight, DWORD fit); //4AB1F0.
		virtual void GetTextureHandle(DWORD& handle); // 4AA8F0; H-RETURN.
		virtual void GetTextureName(std::string& name); // 4AB530; H-RETURN.
		virtual void SetSourceRect(float x, float y, float width, float height); // 4AA920.
		void SetTileMode(DWORD mode); // 4AA910.
		void UpdateDimensions(); // 4AAF10; not the inherited Refresh slot.

		DWORD textureHandle; // +D1C; separately owned handle, not the center's borrowed handle.
		BYTE ownsTexture; // +D20; paddingD21..D23 unwritten.
		LONG pictureWidth, pictureHeight; // +D24/+D28, signed FILD operands.
	};
}
