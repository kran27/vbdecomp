#include "WaterTile.hpp"
#include "Water.hpp"
#include "ModelNode.hpp"
#include "SystemInterface.hpp"
#include <algorithm>

void Gfx_WaterTile::BuildIndices(const char*)
{
	if (indices) { delete[] indices; indices = NULL; }
	Gfx_Water* water = GetWater();
	if (!water) return;
	const LONG sampleWidth = water->sampleWidth, sampleHeight = water->sampleHeight;
	triangleList = 0; primitiveCount = 0;
	bool masked = false;
	const LONG rowBytes = sampleWidth / 8 + (sampleWidth % 8 != 0);
	if (water->sampleMask)
	{
		for (LONG z = 0; z < rows; ++z)
			for (LONG x = 0; x < columns; ++x)
			{
				LONG column = x + offsetX;
				LONG index = rowBytes * (z + offsetZ) + column / 8;
				if (!(water->sampleMask[index] & (1 << (column % 8))))
				{ masked = true; break; }
			}
	}
	if (masked)
	{
		LONG quads = 0;
		const LONG width = (std::min)(columns - 1, sampleWidth);
		const LONG height = (std::min)(rows - 1, sampleHeight);
		for (LONG z = 0; z < height; ++z)
			for (LONG x = 0; x < width; ++x)
				if (x + offsetX < sampleWidth && z + offsetZ < sampleHeight
					&& (water->sampleMask[rowBytes * (z + offsetZ) + (x + offsetX) / 8] & (1 << ((x + offsetX) % 8)))) ++quads;
		if (quads > 0)
		{
			indexCount = DWORD(6 * quads); indices = new WORD[indexCount];
			WORD* output = indices;
			const LONG vertexColumns = (std::min)(columns, sampleWidth);
			const LONG vertexRows = (std::min)(rows, sampleHeight);
			for (LONG z = 0; z < vertexRows; ++z)
				for (LONG x = 0; x < vertexColumns; ++x)
				{
					if (x + offsetX >= sampleWidth || z + offsetZ >= sampleHeight
						|| !(water->sampleMask[rowBytes * (z + offsetZ) + (x + offsetX) / 8] & (1 << ((x + offsetX) % 8)))) continue;
					*output++ = WORD(z * columns + x + 1);
					*output++ = WORD(z * columns + x);
					*output++ = WORD((z + 1) * columns + x);
					*output++ = WORD(z * columns + x + 1);
					*output++ = WORD((z + 1) * columns + x);
					*output++ = WORD((z + 1) * columns + x + 1);
					primitiveCount += 2;
					//The original masked branch writes UVs with the transposed vertex
					//index. Its triangle indices retain ordinary row-major addressing.
					const LONG vertex = z + x * columns;
					if (flag226)
					{
						vertices32[vertex].u = float(float(x) / (columns - 1) * surfaceWidth + surfaceU);
						vertices32[vertex].v = float((1.0f - float(z) / (rows - 1)) * surfaceHeight + surfaceV);
						vertices32[vertex].lightU = float(float(x % (columns - 1)) / (columns - 1));
						vertices32[vertex].lightV = float(1.0f - float(z % (rows - 1)) / (rows - 1));
						vertices32[vertex].lightU = lightWidth * vertices32[vertex].lightU + lightU;
						vertices32[vertex].lightV = lightHeight * vertices32[vertex].lightV + lightV;
					}
					else
					{
						vertices44[vertex].u = float(float(x) / (columns - 1) * surfaceWidth + surfaceU);
						vertices44[vertex].v = float((1.0f - float(z) / (rows - 1)) * surfaceHeight + surfaceV);
						vertices44[vertex].lightU = float(float(x % (columns - 1)) / (columns - 1));
						vertices44[vertex].lightV = float(1.0f - float(z % (rows - 1)) / (rows - 1));
						vertices44[vertex].lightU = lightWidth * vertices44[vertex].lightU + lightU;
						vertices44[vertex].lightV = lightHeight * vertices44[vertex].lightV + lightV;
					}
				}
			triangleList = 1;
		}
		return;
	}

	//510F5C..51116D emits the actual inward spiral triangle strip, retaining
	//the original allocation/count and its unused trailing WORDs.
	indexCount = DWORD(2 * columns * rows);
	indices = new WORD[indexCount];
	WORD* output = indices;
	LONG x = 0, z = 0;
	const LONG firstExtent = (std::min)(columns, rows);
	LONG remaining = firstExtent, width = columns, height = rows;
	while (remaining > 0)
	{
		if (width <= 1) break;
		for (LONG step = 0; step < width; ++step)
		{
			*output++ = WORD(x + z * columns);
			*output++ = WORD(x + (z + 1) * columns);
			++x;
			if (step || remaining != firstExtent) primitiveCount += 2;
		}
		LONG right = x - 1, down = z + 1;
		if (height <= 2) break;
		for (LONG step = height - 1; step > 0; --step)
		{
			*output++ = WORD(right + down * columns);
			*output++ = WORD(right - 1 + down * columns);
			++down; primitiveCount += 2;
		}
		LONG bottom = down - 1, left = right - 1;
		if (width <= 2) break;
		for (LONG step = width - 1; step > 0; --step)
		{
			*output++ = WORD(left + bottom * columns);
			*output++ = WORD(left + (bottom - 1) * columns);
			--left; primitiveCount += 2;
		}
		LONG innerLeft = left + 1, up = bottom - 1;
		if (height <= 3) break;
		for (LONG step = height - 2; step > 0; --step)
		{
			*output++ = WORD(innerLeft + up * columns);
			*output++ = WORD(innerLeft + 1 + up * columns);
			--up; primitiveCount += 2;
		}
		height -= 2; width -= 2; remaining -= 2;
		x = innerLeft + 1; z = up + 1;
	}
	const LONG widthLimit = (std::min)(columns, sampleWidth);
	const LONG heightLimit = (std::min)(rows, sampleHeight);
	for (LONG row = 0; row < heightLimit; ++row)
		for (LONG column = 0; column < widthLimit; ++column)
			if (column + offsetX < sampleWidth && row + offsetZ < sampleHeight)
			{
				const LONG vertex = column + row * columns;
				if (flag226)
				{
					vertices32[vertex].u = float(float(column) / (columns - 1) * surfaceWidth + surfaceU);
					vertices32[vertex].v = float((1.0f - float(row) / (rows - 1)) * surfaceHeight + surfaceV);
					LONG lu = column == widthLimit - 1 ? columns - 1 : column % (columns - 1);
					vertices32[vertex].lightU = float(float(lu) / (columns - 1));
					LONG lv = row == heightLimit - 1 ? rows - 1 : row % (rows - 1);
					vertices32[vertex].lightV = float(1.0f - float(lv) / (rows - 1));
					vertices32[vertex].lightU = lightWidth * vertices32[vertex].lightU + lightU;
					vertices32[vertex].lightV = lightHeight * vertices32[vertex].lightV + lightV;
				}
				else
				{
					vertices44[vertex].u = float(float(column) / (columns - 1) * surfaceWidth + surfaceU);
					vertices44[vertex].v = float((1.0f - float(row) / (rows - 1)) * surfaceHeight + surfaceV);
					LONG lu = column == widthLimit - 1 ? columns - 1 : column % (columns - 1);
					vertices44[vertex].lightU = float(float(lu) / (columns - 1));
					LONG lv = row == heightLimit - 1 ? rows - 1 : row % (rows - 1);
					vertices44[vertex].lightV = float(1.0f - float(lv) / (rows - 1));
					vertices44[vertex].lightU = lightWidth * vertices44[vertex].lightU + lightU;
					vertices44[vertex].lightV = lightHeight * vertices44[vertex].lightV + lightV;
				}
			}
}

void Gfx_WaterTile::BuildPacket()
{
	if (!transforms) return;
	Gfx_Water* water = GetWater();
	if (!water) return;
	mesh.value00 = 0; mesh.value04 = DWORD(-1);
	const DWORD fvf = flag226 ? 0x242 : 0x252;
	mesh.vertices = flag226 ? static_cast<void*>(vertices32) : static_cast<void*>(vertices44);
	mesh.SetVertexDesc(fvf, D3DXGetFVFVertexSize(fvf), vertexCount);
	mesh.value10 = reinterpret_cast<DWORD>(indices);
	mesh.SetIndexDesc(2, indexCount);
	delete packet; packet = new Gfx_PacketData;
	Gfx_Primitive* primitive = mesh.SetPrimitiveCount(1);
	primitive->value00 = 0; primitive->value24 = 1;
	primitive->type = triangleList ? D3DPT_TRIANGLELIST : D3DPT_TRIANGLESTRIP;
	Gfx_Material material;
	material.mode24 = material.mode28 = mode28;
	if (!flag224) material.flagB9 = 0;
	material.flagBC = flag226 == 0;
	material.style1 = material.style2 = water->flag174
		? (lightmap ? 25 : 19) + (flag225 != 0) : (lightmap ? 12 : 8) + (flag225 != 0);
	material.textureHandles[0] = texture; material.textureHandles[1] = lightmap;
	material.flagBD = 1;
	mesh.materialSet.materials[0] = material;
	primitive->value28 = primitive->value2C = 0; primitive->value30 = primitiveCount;
	packet->transformEnabled = 1; packet->transform = transforms;
	if (!flag226)
		for (Gfx_LightList::iterator light = lights.begin(); light != lights.end(); ++light) packet->AddLight(*light);
	mesh.materialSet.RebuildGroups(); mesh.value04 = DWORD(-1);
	instances[0].SetMesh(&mesh);
	instances[0].value0C = reinterpret_cast<DWORD>(packet);
	instances[0].flag1D = 1; instances[0].flag1E = 1;
}

void Gfx_WaterTile::Draw(Gfx_Viewport* viewport)
{
	if (owner)
	{
		transforms[0] = static_cast<Gfx_ModelNode*>(owner)->worldTransform;
		transforms[0]._41 = float(double(transforms[0]._41) - extentX * 0.5);
		transforms[0]._43 = float(double(transforms[0]._43) - extentZ * 0.5);
	}
	else D3DXMatrixIdentity(transforms);
	Gfx_Water* water = GetWater();
	if (!water) return;
	const float cell = water->cellSize;
	transforms[0]._41 = float(double(offsetX) * cell + transforms[0]._41);
	transforms[0]._43 = float(double(offsetZ) * cell + transforms[0]._43);
	D3DXMATRIX rotation = transforms[0];
	rotation._41 = rotation._42 = rotation._43 = 0;
	D3DXMATRIX inverse;
	D3DXMatrixInverse(&inverse, NULL, &rotation);
	D3DXMatrixMultiply(transforms, transforms, &inverse);
	transforms[0]._41 = float(double(transforms[0]._41) - offsetX * cell);
	transforms[0]._43 = float(double(transforms[0]._43) - offsetZ * cell);
	D3DXMatrixMultiply(transforms, transforms, &rotation);
	D3DXMatrixMultiply(transforms, transforms, &extraTransform);
	if (dirty) { BuildPacket(); dirty = 0; }
	if (instances) g_GfxSystem->QueueMeshInstances(instances, viewport, instanceCount);
}
