#pragma once

namespace JE
{
	// Original four-slot owner shared by all eight startup registry components.
	// The startup GameComponent spelling is an alias of this same interface.
	// Original derived construction omits interface vfptr writes and base-only
	// exception cleanup. VC7 novtable preserves that abstract-owner contract.
	struct __declspec(novtable) IComponent
	{
		virtual ~IComponent() {}
		virtual void Initialize() = 0;
		virtual void Cleanup() = 0;
		virtual void Update() = 0;
	};
}
