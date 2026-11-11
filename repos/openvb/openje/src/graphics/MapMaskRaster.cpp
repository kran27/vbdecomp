#include "MapMaskRaster.hpp"
#include "MapMaskManager.hpp"
#include "Texture.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "entity/MapFog.hpp"
#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace
{
	LONG Integer(double value) { return LONG(static_cast<__int64>(value)); }
	DWORD Channel(double value, double maximum) { return DWORD(static_cast<__int64>(value * maximum)); }
	DWORD Pack(double value, D3DFORMAT format, bool flat)
	{
		DWORD c;
		switch (format)
		{
		case D3DFMT_R8G8B8: case D3DFMT_X8R8G8B8:
			c = Channel(value, 255); return c | (c << 8) | (c << 16);
		case D3DFMT_A8R8G8B8:
			c = Channel(value, 255); return c | (c << 8) | (c << 16) | (c << 24);
		case D3DFMT_R5G6B5:
			c = Channel(value, 31); return c | (Channel(value, 63) << 5) | (c << 11);
		case D3DFMT_X1R5G5B5:
			c = Channel(value, 31); return c | (c << 5) | (c << 10);
		case D3DFMT_A1R5G5B5:
			c = Channel(value, 31); return c | (c << 5) | (c << 10) | (DWORD(value != 0) << 15);
		case D3DFMT_A4R4G4B4:
			c = Channel(value, 15); return c | (c << 4) | (c << 8) | (c << 12);
		case D3DFMT_A8R3G3B2:
			if (flat) return Channel(value, 3) | (Channel(value, 7) << 2)
				| (Channel(value, 7) << 5) | (Channel(value, 255) << 8);
			//Actual597129..59715B uses15/7/3 and shifts6 then5 in this branch.
			return Channel(value, 3) | ((Channel(value, 7) | (Channel(value, 15) << 6)) << 5);
		default: return 0; //Only called after the original supported-format selection.
		}
	}
	DWORD PixelBytes(D3DFORMAT format, bool visible)
	{
		switch (format)
		{
		case D3DFMT_R8G8B8: return 3;
		case D3DFMT_X8R8G8B8: return 4;
		case D3DFMT_R5G6B5: case D3DFMT_X1R5G5B5: return 2;
		case D3DFMT_A8R8G8B8: return visible ? 0 : 4;
		case D3DFMT_A1R5G5B5: case D3DFMT_A4R4G4B4: case D3DFMT_A8R3G3B2: return visible ? 0 : 2;
		default: return 0; //Original switch returns without writing unsupported formats.
		}
	}
	void Render(const D3DLOCKED_RECT& lock, const RECT& rectangle, D3DFORMAT format,
		DWORD bytes, float topLeft, float topRight, float bottomLeft, float bottomRight)
	{
		bool flat = topLeft == topRight && topRight == bottomLeft && bottomLeft == bottomRight;
		if (flat)
		{
			DWORD packed = Pack(topLeft, format, true);
			for (LONG y = rectangle.top; y < rectangle.bottom; ++y)
			{
				BYTE* destination = static_cast<BYTE*>(lock.pBits) + y * lock.Pitch + bytes * rectangle.left;
				for (LONG x = rectangle.left; x < rectangle.right; ++x, destination += bytes)
					memcpy(destination, &packed, bytes);
			}
			return;
		}
		double count = rectangle.bottom - rectangle.top;
		float leftStep = float((bottomLeft - topLeft) / count);
		double rightStep = (bottomRight - topRight) / count;
		for (LONG y = rectangle.top, row = 0; y < rectangle.bottom; ++y, ++row)
		{
			float left = float(double(leftStep) * row + topLeft);
			double right = double(row) * rightStep + topRight;
			double step = (right - left) / (rectangle.right - rectangle.left);
			BYTE* destination = static_cast<BYTE*>(lock.pBits) + y * lock.Pitch + bytes * rectangle.left;
			for (LONG x = rectangle.left, column = 0; x < rectangle.right; ++x, ++column, destination += bytes)
			{
				DWORD value = Pack(double(column) * step + left, format, false);
				memcpy(destination, &value, bytes); //Actual1/2/3/4 byte stores, including R8G8B8.
			}
		}
	}
	BYTE CellBit(const JE::AreaMapBits& bits, LONG width, LONG height, LONG x, LONG y)
	{
		return x >= 0 && x < width && y >= 0 && y < height ? MapMaskRaster::GetBit(bits, x + y * width) : 0;
	}
	void Draw(const D3DLOCKED_RECT& lock, const RECT& rectangle, D3DFORMAT format,
		LONG x, LONG y, const JE::AreaMapBits& bits, LONG width, LONG height)
	{
		DWORD bytes = PixelBytes(format, false);
		if (!bytes) return;
		float base = CellBit(bits, width, height, x, y) ? 0.25f : 0;
		float tl = base, tr = base, bl = base, br = base;
		if (CellBit(bits, width, height, x - 1, y)) { tl += 0.25f; bl = tl; }
		if (CellBit(bits, width, height, x - 1, y - 1)) tl += 0.25f;
		if (CellBit(bits, width, height, x, y - 1)) { tl += 0.25f; tr = base + 0.25f; }
		if (CellBit(bits, width, height, x + 1, y)) { tr += 0.25f; br = base + 0.25f; }
		if (CellBit(bits, width, height, x + 1, y - 1)) tr += 0.25f;
		if (CellBit(bits, width, height, x - 1, y + 1)) bl += 0.25f;
		if (CellBit(bits, width, height, x, y + 1)) { bl += 0.25f; br += 0.25f; }
		if (CellBit(bits, width, height, x + 1, y + 1)) br += 0.25f;
		Render(lock, rectangle, format, bytes, tl, tr, bl, br);
	}
}

namespace JE
{
	BYTE AreaMapBits::GetBit(LONG index) const
	{
		const LONG byte = index / 8;
		if (byte >= LONG(size)) return 0;
		const BYTE mask = BYTE(1u << (index % 8));
		return (data[byte] & mask) ? 1 : 0;
	}
	void AreaMapBits::SetBit(LONG index, BYTE value)
	{
		LONG byte = index / 8;
		if (byte >= LONG(size))
		{
			data = static_cast<BYTE*>(realloc(data, byte + 1));
			memset(data + size, 0, byte + 1 - size);
			size = byte + 1;
		}
		BYTE mask = BYTE(1 << (index % 8));
		if (value) data[byte] |= mask;
		else data[byte] &= ~mask;
	}

}

namespace MapMaskRaster
{
	BYTE GetBit(const JE::AreaMapBits& bits, LONG index)
	{
		return bits.GetBit(index);
	}
	void SetBit(JE::AreaMapBits& bits, LONG index, BYTE value)
	{
		bits.SetBit(index, value);
	}
}

BYTE MapMaskManager::CellRectangle(const JE::AreaMap* area, Gfx_Texture* texture,
		LONG column, LONG row, RECT& output) //596890.
{
	if (!texture || !area || column < 0) return 0;
	const LONG columns = area->width;
	if (column >= columns || row < 0) return 0;
	const LONG rows = area->height;
	if (row >= rows) return 0;

	const LONG frameCount = texture->frameCount;
	const LONG textureWidth = frameCount > 0 ? texture->frames[0].width : 0;
	const LONG textureHeight = frameCount > 0 ? texture->frames[0].height : 0;
	const float left = float(double(column) * textureWidth / columns);
	const float top = float(double(row) * float(textureHeight) / float(rows));
	const double right = double(column + 1) * textureWidth / columns;
	const float bottom = float(double(row + 1) * float(textureHeight) / float(rows));

	output.left = Integer(left);
	output.top = Integer(top);
	float integral;
	const float spanX = float(right - left);
	const LONG extraX = modff(spanX, &integral) != 0.0f;
	output.right = output.left + Integer(spanX) + extraX;
	const float spanY = bottom - top;
	const LONG extraY = modff(spanY, &integral) != 0.0f;
	output.bottom = output.top + Integer(spanY) + extraY;
	return 1;
}

void MapMaskManager::DrawExplored(Gfx_Texture* texture, const D3DLOCKED_RECT& lock, D3DFORMAT format,
		LONG column, LONG row, const JE::AreaMap& area)
{
	RECT rectangle;
	if (!CellRectangle(&area, texture, column, row, rectangle)) return;
	Draw(lock, rectangle, format, column, row, area.explored, area.width, area.height);
}

void MapMaskManager::DrawVisible(Gfx_Texture* texture, const D3DLOCKED_RECT& lock, D3DFORMAT format,
		LONG column, LONG row, const JE::AreaMapBits& bits, LONG width, LONG height) //5972E0.
{
	JE::IWorldClient* client = JE::g_World;
	const DWORD mapID = client->GetCurrentMapID();
	if (!mapID) return;
	JE::IWorld* world = JE::g_World;
	JE::Map* map = world->GetMap(mapID);
	if (!map) return;
	const JE::AreaMap* area = map->areaMap;
	if (!area) return;

	RECT rectangle;
	if (!CellRectangle(area, texture, column, row, rectangle)) return;
	const DWORD bytes = PixelBytes(format, true);
	if (!bytes) return;

	const LONG center = column + row * width;
	const LONG left = column - 1;
	const LONG right = column + 1;
	const LONG above = row - 1;
	const LONG below = row + 1;
	const float base = bits.GetBit(center) ? 0.25f : 0.0f;
	float tl = base, tr = base, bl = base, br = base;
	if (left >= 0 && bits.GetBit(center - 1)) { tl += 0.25f; bl = tl; }
	if (left >= 0 && above >= 0 && bits.GetBit(left + above * width)) tl += 0.25f;
	if (above >= 0 && bits.GetBit(column + above * width)) { tl += 0.25f; tr = base + 0.25f; }
	if (right < width && bits.GetBit(center + 1)) { tr += 0.25f; br = base + 0.25f; }
	if (right < width && above >= 0 && bits.GetBit(right + above * width)) tr += 0.25f;
	if (left >= 0 && below < height && bits.GetBit(left + below * width)) bl += 0.25f;
	if (below < height && bits.GetBit(column + below * width)) { bl += 0.25f; br += 0.25f; }
	if (right < width && below < height && bits.GetBit(right + below * width)) br += 0.25f;
	Render(lock, rectangle, format, bytes, tl, tr, bl, br);
}
