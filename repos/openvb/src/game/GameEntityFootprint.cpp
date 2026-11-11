#include "GameCreature.hpp"
#include "entity/SpatialFootprint.hpp"
#include <stdlib.h>
#include <new>

namespace F3
{
	JE::RLEPrimitive<JE::Entity*>* GameCreature::CreateSpatialFootprint()
	{
		// Actual40D840 uses the Entity bounds (+10/+1C), not model-local bounds.
		D3DXVECTOR3 low = minimum, high = maximum;
		float footprintAngle = rotation.x; //40D884..40D8A2, before allocation.
		if (high.x - low.x < 4.0f) { low.x = 0.0f; high.x = 4.0f; }
		if (high.z - low.z < 4.0f) { low.z = 0.0f; high.z = 4.0f; }
		LONG offsetX = labs(static_cast<LONG>(static_cast<__int64>(low.x)));
		float offsetZ = static_cast<float>(labs(static_cast<LONG>(static_cast<__int64>(low.z))));
		LONG left = static_cast<LONG>(static_cast<__int64>(double(offsetX) + low.x));
		LONG right = static_cast<LONG>(static_cast<__int64>(double(offsetX) + high.x));
		LONG bottom = static_cast<LONG>(static_cast<__int64>(double(offsetZ) + low.z));
		LONG top = static_cast<LONG>(static_cast<__int64>(double(offsetZ) + high.z));
		return new JE::RLEConvexQuad<JE::Entity*>(left, bottom, left, top,
			right, top, right, bottom, footprintAngle, this);
	}
}
