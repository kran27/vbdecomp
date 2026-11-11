#include "MapFog.hpp"
#include "graphics/SystemInterface.hpp"
#include <math.h>
#include <stdlib.h>
#include <cstring>
#include <stdexcept>

namespace JE
{
	float AreaMap::GetCellSpacing() const { return 2.0f; } //57BEF0, real out-of-line caller boundary.

	BYTE AreaMap::IsExplored(LONG first, LONG row) const //57BC90.
	{
		if (first<0 || first>=width || row<0 || row>=height) return 0;
		return explored.GetBit(first+row*width);
	}
	BYTE AreaMap::SetExplored(LONG first, LONG row, BYTE value) //57BCD0.
	{
		if (first<0 || first>=width || row<0 || row>=height) return 0;
		LONG bit=first+row*width;
		if ((explored.GetBit(bit) ? 1 : 0)==(value ? 1 : 0)) return 0;
		explored.SetBit(bit,value);
		changes|=1;
		return 1;
	}

	void AreaMap::WorldToCell(float x, float, float z, LONG* first, LONG* row) const
	{
		float spanX=maxX-minX,spanZ=maxZ-minZ;
		if(spanX==0 || spanZ==0) { *first=*row=-1; return; }
		*first=LONG(width/spanX*(x-minX));
		*row=height-LONG((z-minZ)/spanZ*height);
	}
	void AreaMap::WorldToCell(const D3DXVECTOR3& position,LONG* first,LONG* row) const
	{
		WorldToCell(position.x,position.y,position.z,first,row);
	}
	BYTE AreaMap::IsVisible(LONG first,LONG row) const
	{
		if(first<0 || first>=width || row<0 || row>=height) return 0;
		const LONG bit=LONG(DWORD(first)+DWORD(row)*DWORD(width));
		return visible.GetBit(bit);
	}
	BYTE AreaMap::SetVisible(LONG first, LONG row, BYTE value)
	{
		if (first < 0 || first >= width || row < 0 || row >= height) return 0;
		const LONG bit = LONG(DWORD(first) + DWORD(row) * DWORD(width));
		visible.SetBit(bit, value);
		return 1;
	}

	void AreaMapBits::Clear()
	{
		if (data)
		{
			free(data);
			data = NULL;
		}
		size = 0;
	}

	AreaMapNote& AreaMapNote::operator=(const AreaMapNote& other)
	{
		if (texture)
			g_GfxSystem->DeleteTexture(texture);
		texture = 0;
		textureWidth = 0;
		textureHeight = 0;
		name = other.name;
		value20 = other.value20;
		value24 = other.value24;
		value28 = other.value28;
		value2C = other.value2C;
		value30 = other.value30;
		text = other.text;
		position[0] = other.position[0];
		position[1] = other.position[1];
		position[2] = other.position[2];
		flag5C = other.flag5C;
		flag5D = other.flag5D;
		return *this;
	}
	void AreaMapNote::ClearTexture()
	{
		if (texture)
		{
			g_GfxSystem->DeleteTexture(texture);
			texture = 0;
		}
	}

	AreaMap::AreaMap(DWORD firstValue, DWORD secondValue, const std::string& name,
		float firstX, float firstZ, float lastX, float lastZ, LONG viewWidth, LONG viewHeight)
	{
		value4 = firstValue;
		value8 = secondValue;
		textureName = name;
		minX = firstX;
		minZ = firstZ;
		maxX = lastX;
		maxZ = lastZ;
		screenWidth = viewWidth;
		screenHeight = viewHeight;
		float spanX = lastX - firstX;
		float spanZ = lastZ - firstZ;
		// Original uses fmod/FTOL, not a positive-size clamp or generic ceil.
		width = (fmod(spanX, 2.0) != 0.0) - static_cast<LONG>(spanX * -0.5f);
		height = (fmod(spanZ, 2.0) != 0.0) - static_cast<LONG>(spanZ * -0.5f);
		changes = 0;
		heightsDirty = 0;
	}
	AreaMap::~AreaMap() {}
	LONG AreaMap::AddNote(const std::string& name, DWORD value20, DWORD value24,
		DWORD value28, DWORD value2C, LONG value30, const float* position, BYTE flag) //57FE20.
	{
		AreaMapNote note;
		note.name=name; note.value20=value20; note.value24=value24;
		note.value28=value28; note.value2C=value2C; note.value30=value30;
		for (DWORD axis=0;axis<3;++axis) note.position[axis]=position[axis];
		note.flag5C=flag;
		notes.push_back(note);
		changes|=2;
		return LONG(notes.size())-1;
	}
	AreaMapNote* AreaMap::FindNote(LONG index) //41F2B0.
	{
		if (index < 0 || index >= LONG(notes.size())) return NULL;
		return &notes[index];
	}
	LONG AreaMap::AddNote(const std::string& name, DWORD value20, DWORD value24,
		DWORD value28, DWORD value2C, const std::string& text, const float* position, BYTE flag) //57FCF0.
	{
		AreaMapNote note;
		note.name = name;
		note.value20 = value20;
		note.value24 = value24;
		note.value28 = value28;
		note.value2C = value2C;
		note.text = text;
		std::memcpy(note.position, position, sizeof(note.position));
		note.flag5C = flag;
		notes.push_back(note);
		changes |= 2;
		return LONG(notes.size())-1;
	}
	BYTE AreaMap::RemoveNote(LONG index) //57E3F0.
	{
		if (index < 0 || index >= LONG(notes.size())) return 0;
		notes.erase(notes.begin()+index);
		changes |= 2;
		return 1;
	}
	void AreaMap::ResetHeights()
	{
		heights.clear();
		derivedHeights.clear();
		rectangles.clear();
		heightLists.clear();
		layers.clear();
		heightsDirty = 1;
		if (width > 0 && height > 0) heights.insert(heights.begin(), width * height, 0.0f);
	}
	void AreaMap::ResetSnapshot()
	{
		visible.Clear();
	}
	void AreaMap::CellToWorld(LONG first, LONG row, float* x, float* z, BYTE center)
	{
		if (!width || !height) return;
		*x = static_cast<float>(first) / width * (maxX - minX) + minX;
		*z = static_cast<float>(height - row) / height * (maxZ - minZ) + minZ;
		if (center)
		{
			*x += (maxX - minX) * 0.5f / width;
			*z -= (maxZ - minZ) * 0.5f / height;
		}
	}
	BYTE AreaMap::SetHeight(LONG first, LONG row, float value)
	{
		if (first < 0 || first >= width || row < 0 || row >= height) return 0;
		heights[first + row * width] = value;
		heightsDirty = 1;
		return 1;
	}
	void AreaMap::AddLayerHeight(LONG layer, LONG first, LONG row, float value)
	{
		AreaMapHeight cell;
		cell.first = first;
		cell.row = row;
		cell.height = value;
		layers[layer].cells.push_back(cell);
		heightsDirty = 1;
	}
	BYTE AreaMap::SetHeightList(DWORD id, const std::list<AreaMapHeight>& cells)
	{
		typedef std::map<DWORD,std::list<AreaMapHeight> > HeightLists;
		HeightLists::iterator found=heightLists.find(id);
		std::list<AreaMapHeight>* list;
		if (found==heightLists.end()) list=&heightLists[id];
		else list=&found->second;
		*list=cells;
		heightsDirty = 1;
		return 1;
	}
	BYTE AreaMap::SetHeightRectangle(DWORD id, float lowX, float lowY, float lowZ,
		float highX, float highY, float highZ)
	{
		LONG x1, row1, x2, row2;
		WorldToCell(lowX, lowY, lowZ, &x1, &row1);
		WorldToCell(highX, highY, highZ, &x2, &row2);
		typedef std::map<DWORD,AreaMapRectangle> Rectangles;
		Rectangles::iterator found=rectangles.find(id);
		if (found==rectangles.end())
		{
			AreaMapRectangle rectangle;
			found=rectangles.insert(Rectangles::value_type(id,rectangle)).first;
		}
		AreaMapRectangle& value=found->second;
		value.first = x1 < x2 ? x1 : x2;
		value.row = row1 < row2 ? row1 : row2;
		value.last = x1 > x2 ? x1 : x2;
		value.lastRow = row1 > row2 ? row1 : row2;
		value.height = lowY > highY ? lowY : highY;
		heightsDirty = 1;
		return 1;
	}


}
