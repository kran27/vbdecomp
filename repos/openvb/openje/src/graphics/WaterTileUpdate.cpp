#include "WaterTile.hpp"
#include "Water.hpp"
#include "WaterSampling.hpp"
#include "ParticleSystem.hpp"
#include "ModelNode.hpp"
#include <algorithm>
#include <cmath>

void Gfx_WaterTile::Update(float time) //511570.
{
	Gfx_Water* water = GetWater();
	if (!water || water->forceUpdate) return;
	double elapsed = 0;
	if (lastTime) elapsed = double(time) - *lastTime;
	if (lastTime && elapsed < 0)
	{
		accumulatedTime = 0;
		delete lastTime; lastTime = NULL;
	}
	else accumulatedTime = float(elapsed + accumulatedTime);
	if (!lastTime || accumulatedTime >= value23C)
	{
		accumulatedTime = 0;
		DWORD currentAlpha = alpha;
		if (animateAlpha)
		{
			double phaseTime = alphaDuration > 0 && double(time) / alphaDuration >= 0
				? (std::min)(double(time) / alphaDuration, 1.0) : 0;
			if (!normalizeAlphaTime || alphaDuration <= 0) phaseTime = time;
			float input = float(phaseTime * alphaFrequency + alphaPhase);
			double wave = Gfx_ParticleWaveform(LONG(alphaWaveform), 0, 1, input);
			currentAlpha = DWORD(static_cast<LONG>((LONG(alphaEnd) - LONG(alpha)) * wave + alpha));
		}
		const LONG width = water->sampleWidth, height = water->sampleHeight;
		const LONG sampleCount = width * height;
		const float cell = water->cellSize;
		const double extent = (std::max)(width, height) * double(cell);
		const float sphereHeight = float(0.5714286 * extent);
		const float radiusSquared = float(extent * extent);
		const float farY = float(extent * -6400.0);
		float zPosition = 0;
		for (LONG row = 0; row < rows; ++row)
		{
			const LONG centerRow = width * (row + offsetZ);
			LONG previousRow = centerRow - width;
			if (previousRow < 0) previousRow += sampleCount;
			LONG nextRow = centerRow + width;
			if (nextRow >= sampleCount) nextRow -= sampleCount;
			float xPosition = 0;
			for (LONG column = 0; column < columns; ++column)
			{
				const LONG sourceX = column + offsetX;
				LONG previousX = sourceX - 1;
				if (previousX < 0) previousX = width - 1;
				LONG nextX = sourceX + 1;
				if (nextX > width) nextX = 0; //Original51181C is signed>, not>=.
				//511837..511859 uses the previous-row/previous-column sample
				//for displacement as well as the first diagonal gradient.
				const LONG source = previousX + previousRow;
				const Gfx_WaterSample& sample = water->samples[source];
				const float displacement = float(double(sample.height) + sample.velocity - water->meanHeight);
				LONG first = previousX + previousRow, second = nextX + nextRow;
				float nx = 0, nz = 0;
				//The original explicitly writes both zeros at511899/5118A1
				//when either first diagonal index is outside the sample grid.
				if (first >= 0 && first < sampleCount && second >= 0 && second < sampleCount)
					nx = nz = float((double(water->samples[first].height) - water->samples[second].height) / cell);
				first = sourceX + previousRow; second = sourceX + nextRow;
				if (first >= 0 && first < sampleCount && second >= 0 && second < sampleCount)
					nz = float((double(water->samples[first].height) - water->samples[second].height) / cell + nz);
				first = nextX + previousRow; second = previousX + nextRow;
				if (first >= 0 && first < sampleCount && second >= 0 && second < sampleCount)
				{
					double gradient = (double(water->samples[first].height) - water->samples[second].height) / cell;
					nx = float(nx - gradient); nz = float(nz + gradient);
				}
				first = previousX + centerRow; second = nextX + centerRow;
				if (first >= 0 && first < sampleCount && second >= 0 && second < sampleCount)
					nx = float((double(water->samples[first].height) - water->samples[second].height) / cell + nx);
				D3DXVECTOR3 normal(nx, 1, nz);
				D3DXVec3Normalize(&normal, &normal);
				D3DXVECTOR3 incident(-xPosition, farY - displacement, -zPosition);
				D3DXVec3Normalize(&incident, &incident);
				const float dot = float((double(incident.z) * normal.z + double(incident.y) * normal.y + double(incident.x) * normal.x) * 255.0);
				LONG lookup = (static_cast<LONG>(dot) + 256) % 512;
				if (lookup < 0) lookup += 512;
				const Gfx_WaterRefraction& sampleRefraction = water->refraction[lookup];
				const LONG output = column + row * columns;
				const DWORD color = blue | ((green | (((currentAlpha << 8) | red) << 8)) << 8);
				if (flag226)
				{
					Gfx_WaterVertex32& vertex = vertices32[output];
					vertex.x = xPosition; vertex.y = displacement; vertex.z = zPosition; vertex.color = color;
				}
				else
				{
					Gfx_WaterVertex44& vertex = vertices44[output];
					vertex.x = xPosition; vertex.y = displacement; vertex.z = zPosition;
					vertex.nx = normal.x; vertex.ny = normal.y; vertex.nz = normal.z; vertex.color = color;
				}
				const float refractX = float((normal.x * double(sampleRefraction.x) + incident.x) * sampleRefraction.y);
				const float refractY = float((normal.y * double(sampleRefraction.x) + incident.y) * sampleRefraction.y);
				const float refractZ = float((normal.z * double(sampleRefraction.x) + incident.z) * sampleRefraction.y);
				const double middle = (width - 1) * double(cell) * 0.5;
				const float relativeX = float(xPosition - middle), relativeZ = float(zPosition - middle);
				const float sphereTerm = float(double(relativeZ) * relativeZ + double(relativeX) * relativeX
					+ (double(displacement) - sphereHeight) * (double(displacement) - sphereHeight) - radiusSquared);
				double mappedX = xPosition, mappedZ = zPosition;
				if (sphereTerm < 0)
				{
					double projection = double(refractY) * displacement + double(refractX) * xPosition + double(refractZ) * zPosition;
					double distance = (std::sqrt((projection + projection) * (projection + projection) - sphereTerm * 4.0)
						- (projection + projection)) * 0.5;
					mappedX = refractX * distance + xPosition;
					mappedZ = refractZ * distance + zPosition;
				}
				const float scaledX = float(mappedX * water->inverseWidth);
				const double scaledZ = 1.0 - water->inverseHeight * mappedZ;
				const float u = float(scaledX * double(surfaceWidth) + surfaceU);
				const float v = float(scaledZ * surfaceHeight + surfaceV);
				if (flag226)
				{
					vertices32[output].u = u; vertices32[output].v = v;
					if (JE::GetWaterLightmapSampling())
					{ vertices32[output].lightU = float(scaledX * double(lightWidth) + lightU); vertices32[output].lightV = float(scaledZ * lightHeight + lightV); }
				}
				else
				{
					vertices44[output].u = u; vertices44[output].v = v;
					if (JE::GetWaterLightmapSampling())
					{ vertices44[output].lightU = float(scaledX * double(lightWidth) + lightU); vertices44[output].lightV = float(scaledZ * lightHeight + lightV); }
				}
				xPosition = float(double(xPosition) + cell);
			}
			zPosition = float(double(zPosition) + cell);
		}
		if (owner) transforms[0] = static_cast<Gfx_ModelNode*>(owner)->worldTransform;
		dirty = 1;
	}
	if (!lastTime) lastTime = new float;
	*lastTime = time;
}
