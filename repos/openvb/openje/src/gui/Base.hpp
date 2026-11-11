#pragma once

#include <windows.h>
#include <cstdio>

namespace JE { namespace Resources { struct ResourceFileHandle; } }

namespace GUI
{
	// Original primary Base interface at 0x66E41C: fifteen actual slots.
	// The two depth operations are pure in the original table. This class
	// does not stand in for Window's additional slots or Manager lifetime.
	class Base
	{
	public:
		Base(); // 0x4B2720.
		virtual ~Base() {} // Shared compiler-emitted cleanup at0x4AEFF0.
		virtual void SetPosition(LONG x, LONG y); // +04, 0x4AF060.
		virtual void GetPosition(LONG& x, LONG& y); // +08, 0x4B2750.
		virtual void Offset(LONG x, LONG y); // +0C, 0x4B2770.
		virtual void Resize(LONG width, LONG height); // +10, 0x4B2790.
		virtual void GetSize(LONG& width, LONG& height); // +14, 0x4B27C0.
		virtual bool Contains(LONG x, LONG y); // +18, 0x4B27E0; callers consume AL.
		virtual BYTE ReadResource(JE::Resources::ResourceFileHandle* file, int version); // +1C, 0x4B2850.
		virtual BYTE ReadFile(FILE* file, int version); // +20, 0x4B2800.
		virtual BYTE WriteFile(FILE* file); // +24, 0x4B2830.
		virtual void Refresh(); // +28, 0x4B2880.
		virtual void Draw(); // +2C, shared empty 0x404B70.
		virtual float SetDepth(float depth, float increment) = 0; // +30, original purecall.
		virtual float GetDepth() = 0; // +34, original purecall.
		virtual void UpdateGeometry(); // +38, 0x4B2890.

		BYTE geometryDirty; // +04; original padding +05..07 is not initialized.
		RECT rectangle; // +08; sizeof(Base)==0x18 under x86 VC7.
	};
}

// Namespace alias for GUI::Base.
namespace JE { namespace GUI { using ::GUI::Base; } }
