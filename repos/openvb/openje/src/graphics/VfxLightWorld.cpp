#include "VfxLight.hpp"
#include "Octree.hpp"
#include "ParticleSystem.hpp"
#include <string.h>

namespace JE
{
	void VFX_Light::UpdateInfluence(OCTREE* map)
	{
		if (!map) return;
		float phase;
		if (data.duration > 0)
		{
			phase = currentTime / data.duration;
			if (phase < 0) phase = 0;
			else if (phase > 1) phase = 1;
		}
		else phase = 0;
		float range = data.range;
		if (data.adjustRange)
		{
			if (!data.adjustRangeOverLifetime || !(data.duration > 0)) phase = currentTime;
			const float argument = float(phase * data.rangeFreq + data.rangePhase);
			const double amount = Gfx_ParticleWaveform(data.rangeWaveform, 0, 1, argument);
			range = float((data.finalRange - data.range) * amount + data.range);
		}
		if (data.lightType == 2) return; // Directional light keeps any earlier shape untouched.
		if (data.lightType == 1) range *= 0.5f;
		if (influenceShape)
		{
			OCTREE_Crcl2D* circle = dynamic_cast<OCTREE_Crcl2D*>(influenceShape);
			if (circle) circle->SetRadius(range * scale);
			else
			{
				influenceShape->Release();
				influenceShape = NULL;
			}
		}
		if (!influenceShape)
		{
			OctreeShapeConfiguration configuration;
			// value0 is genuinely unwritten; CreateShape's circle branch never reads it.
			configuration.radius = range * scale;
			configuration.type = 1;
			map->CreateShape(&configuration, &influenceShape); // Original ignores its result.
		}
	}

	void VFX_Light::PositionInfluence(OCTREE* map)
	{
		if (!influenceShape || !map) return;
		D3DXVECTOR3 center(transform._41, transform._42, transform._43);
		if (data.lightType == 1)
		{
			float phase = 0;
			if (data.duration > 0)
			{
				phase = currentTime / data.duration;
				if (phase < 0) phase = 0;
				else if (phase > 1) phase = 1;
			}
			float range = data.range;
			if (data.adjustRange)
			{
				if (!data.adjustRangeOverLifetime || !(data.duration > 0)) phase = currentTime;
				const float argument = float(phase * data.rangeFreq + data.rangePhase);
				const double amount = Gfx_ParticleWaveform(data.rangeWaveform, 0, 1, argument);
				range = float((data.finalRange - data.range) * amount + data.range);
			}
			center = D3DXVECTOR3(transform._41, transform._42, transform._43);
			D3DXVECTOR3 direction;
			memcpy(&direction, &data.direction, sizeof(direction));
			D3DXVec3Normalize(&direction, &direction);
			direction *= range * 0.5f;
			center += direction;
		}
		else if (data.lightType == 2) return;
		influenceShape->SetPosition(&center.x);
		map->UpdateShape(influenceShape); // Actual map slot39/+9C,0x5B0880.
	}

	OctreeShapeNodes* VFX_Light::GetInfluenceNodes() const
	{
		return influenceShape ? influenceShape->GetNodes() : NULL;
	}
}
