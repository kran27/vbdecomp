#pragma once

#include "graphics/MeshData.hpp"
#include <vector>

class Gfx_PacketData;
class Gfx_Viewport;
namespace GUI
{
	struct LineSegment
	{
		LONG x1,y1,x2,y2;
		BYTE r1,g1,b1,a1,r2,g2,b2,a2;
	};
	struct LineVertex
	{
		float x,y,z,rhw;
		DWORD color;
	};
	class LineSystem
	{
	public:
		LineSystem(); //4AEDD0.
		virtual ~LineSystem(); //4AE630.
		virtual void Update(); //4AECA0.
		virtual void Draw(Gfx_Viewport* viewport); //4AE120.
		virtual float SetDepth(float depth,float increment); //4ADD40; increment unused.
		virtual float GetDepth(); //4ADD80.
		void Clear(); //4AE280.
		void SetColor(BYTE red,BYTE green,BYTE blue,BYTE alpha); //4AE100.
		void SetClipRectangle(const RECT* rectangle); //4AE0A0.
		void AddLine(LONG x1,LONG y1,LONG x2,LONG y2); //41D060.
		void AddLine(LONG x1,LONG y1,LONG x2,LONG y2,BYTE r1,BYTE g1,BYTE b1,BYTE a1,
			BYTE r2,BYTE g2,BYTE b2,BYTE a2); //4AED50.
		void BuildGeometry(); //4AE730.
		bool ClipLine(LONG& x1,LONG& y1,LONG& x2,LONG& y2); //4ADDA0.

		BYTE red,green,blue,alpha; //+04..07.
		DWORD renderGroup; //+08, actual material.mode28; not a clipping flag.
		std::vector<LineSegment> segments; //Original vector+0C.
		LONG offsetX,offsetY; //+1C/+20.
		RECT* clipRectangle; //+24, owned optional copy.
		BYTE dirty,scaleToResolution; //+28/+29.
		Gfx_MeshData mesh; //+2C, actual owned mesh and material set.
		Gfx_MeshInstance* instances; //+98, owned array.
		Gfx_PacketData* packet; //+9C, actual owner.
		D3DXMATRIX transform; //+A0.
		DWORD instanceCount; //+E0.
		LineVertex* vertices; DWORD vertexCount,vertexCapacity; //+E4/+E8/+EC.
		WORD* indices; DWORD indexCount,indexCapacity; //+F0/+F4/+F8.
		BYTE geometryReady; //+FC; original ctor leaves unwritten until Update.
	private:
		LineSystem(const LineSystem&);
		LineSystem& operator=(const LineSystem&);
	};
}
