#include "Octree.hpp"
#include <new>
#include "OctreeInput.hpp"
#include "OctreeResourceCallbacks.hpp"

OctreeVertexStreams::OctreeVertexStreams()
	: count(0), positions(0), normals(0), colors(0), textureUV(0), lightmapUV(0)
{
}

OctreeVertexStreams::~OctreeVertexStreams()
{
	if (positions) delete[] positions;
	positions = NULL;
	if (normals) delete[] normals;
	normals = NULL;
	if (colors) delete[] colors;
	colors = NULL;
	if (textureUV) delete[] textureUV;
	textureUV = NULL;
	if (lightmapUV) delete[] lightmapUV;
	lightmapUV = NULL;
}

int OctreeVertexStreams::ReadVertexStreams(FILE* stream, WORD attributes)
{
	DWORD header[2];
	if (g_OctreeReadCallback(header, 4, 2, stream) != 2)
	{
	BadHeader:
		if (positions) delete[] positions;
		if (textureUV) delete[] textureUV;
		if (lightmapUV) delete[] lightmapUV;
		return 1;
	}
	if (header[0] != 0x42585456) // VTXB.
	{
		g_OctreeSeekCallback(stream, -8, SEEK_CUR);
		goto BadHeader;
	}
	DWORD wanted = attributes & 0x1F00;
	if (!wanted)
	{
		g_OctreeSeekCallback(stream, header[1], SEEK_CUR);
		return 0;
	}
	DWORD remaining = header[1];
	g_OctreeReadCallback(&count, 4, 1, stream);
	remaining -= 4;
	while (remaining)
	{
		g_OctreeReadCallback(header, 4, 2, stream);
		remaining -= 8;
		switch (header[0])
		{
		case 0x205A5958: // XYZ .
			if (!(wanted & 0x100)) goto Skip;
			positions = new float[3 * count];
			g_OctreeReadCallback(positions, 12, count, stream);
			wanted ^= 0x100;
			break;
		case 0x4C4D524E: // NRML.
			if (!(wanted & 0x200)) goto Skip;
			normals = new float[3 * count];
			g_OctreeReadCallback(normals, 12, count, stream);
			wanted ^= 0x200;
			break;
		case 0x55464944: // DIFU.
			if (!(wanted & 0x400)) goto Skip;
			colors = new DWORD[count];
			g_OctreeReadCallback(colors, 4, count, stream);
			wanted ^= 0x400;
			break;
		case 0x56555854: // TXUV.
			if (!(wanted & 0x800)) goto Skip;
			textureUV = new float[2 * count];
			g_OctreeReadCallback(textureUV, 8, count, stream);
			wanted ^= 0x800;
			break;
		case 0x56554D4C: // LMUV.
			if (!(wanted & 0x1000)) goto Skip;
			lightmapUV = new float[2 * count];
			g_OctreeReadCallback(lightmapUV, 8, count, stream);
			wanted ^= 0x1000;
			break;
		default:
		Skip:
			g_OctreeSeekCallback(stream, header[1], SEEK_CUR);
		}
		remaining -= header[1];
		if (!wanted)
		{
			if (remaining) g_OctreeSeekCallback(stream, remaining, SEEK_CUR);
			return 0;
		}
	}
	return 0;
}

int OctreeVertexStreams::ReadVertexInput(OctreeInput& stream, WORD attributes)
{
	DWORD header[2];
	if (OctreeRead(header, 4, 2, stream) != 2)
	{
	BadHeader:
		delete[] positions;
		delete[] textureUV;
		delete[] lightmapUV;
		return 1;
	}
	if (header[0] != 0x42585456) // VTXB.
	{
		OctreeSeek(stream, -8, SEEK_CUR);
		goto BadHeader;
	}
	DWORD wanted = attributes & 0x1F00;
	if (!wanted)
	{
		OctreeSeek(stream, header[1], SEEK_CUR);
		return 0;
	}
	OctreeRead(&count, 4, 1, stream);
	DWORD remaining = header[1] - 4;
	while (remaining)
	{
		OctreeRead(header, 4, 2, stream);
		remaining -= 8;
		switch (header[0])
		{
		case 0x205A5958: // XYZ .
			if (!(wanted & 0x100)) goto Skip;
			positions = new float[3 * count];
			OctreeRead(positions, 12, count, stream);
			wanted ^= 0x100;
			break;
		case 0x4C4D524E: // NRML.
			if (!(wanted & 0x200)) goto Skip;
			normals = new float[3 * count];
			OctreeRead(normals, 12, count, stream);
			wanted ^= 0x200;
			break;
		case 0x55464944: // DIFU.
			if (!(wanted & 0x400)) goto Skip;
			colors = new DWORD[count];
			OctreeRead(colors, 4, count, stream);
			wanted ^= 0x400;
			break;
		case 0x56555854: // TXUV.
			if (!(wanted & 0x800)) goto Skip;
			textureUV = new float[2 * count];
			OctreeRead(textureUV, 8, count, stream);
			wanted ^= 0x800;
			break;
		case 0x56554D4C: // LMUV.
			if (!(wanted & 0x1000)) goto Skip;
			lightmapUV = new float[2 * count];
			OctreeRead(lightmapUV, 8, count, stream);
			wanted ^= 0x1000;
			break;
		default:
		Skip:
			OctreeSeek(stream, header[1], SEEK_CUR);
		}
		remaining -= header[1];
		if (!wanted)
		{
			if (remaining) OctreeSeek(stream, remaining, SEEK_CUR);
			return 0;
		}
	}
	return 0;
}
