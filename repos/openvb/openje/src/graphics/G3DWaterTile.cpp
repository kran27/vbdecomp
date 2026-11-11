#include "G3DWaterTile.hpp"
#include "TextParser.hpp"

G3D_WaterTile::G3D_WaterTile()
	: offsetX(0), offsetZ(0), columns(0), rows(0), sourceVersion(0)
{
	kind = 26;
}

void G3D_WaterTile::Read(const BYTE* data, DWORD& offset) //4DD120.
{
	G3D_Mesh::Read(data, offset);
	offsetX = *reinterpret_cast<const LONG*>(data + offset);
	offset += 4;
	offsetZ = *reinterpret_cast<const LONG*>(data + offset);
	offset += 4;
	columns = *reinterpret_cast<const LONG*>(data + offset);
	offset += 4;
	rows = *reinterpret_cast<const LONG*>(data + offset);
	offset += 4;
}

void G3D_WaterTile::Read(FILE* stream) //4DD0C0.
{
	G3D_Mesh::Read(stream);
	fread(&offsetX, 4, 1, stream);
	fread(&offsetZ, 4, 1, stream);
	fread(&columns, 4, 1, stream);
	fread(&rows, 4, 1, stream);
}

void G3D_WaterTile::Write(FILE* stream) //4D6C10.
{
	G3D_Mesh::Write(stream);
	fwrite(&offsetX, 4, 1, stream);
	fwrite(&offsetZ, 4, 1, stream);
	fwrite(&columns, 4, 1, stream);
	fwrite(&rows, 4, 1, stream);
}

void G3D_WaterTile::ReadText() //4DD060.
{
	G3DText::Expect('(');
	offsetX = static_cast<LONG>(G3DText::ReadNumber());
	offsetZ = static_cast<LONG>(G3DText::ReadNumber());
	columns = static_cast<LONG>(G3DText::ReadNumber());
	rows = static_cast<LONG>(G3DText::ReadNumber());
	G3DText::Expect(')');
	G3D_Mesh::ReadText();
}

void G3D_WaterTile::WriteText(FILE* stream) //4D6BC0.
{
	fprintf(stream, "%s( %d %d %d %d )\n", "WaterTile", offsetX, offsetZ, columns, rows);
	G3D_Mesh::WriteText(stream);
}
