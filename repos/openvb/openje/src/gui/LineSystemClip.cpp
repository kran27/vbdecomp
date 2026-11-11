#include "gui/LineSystem.hpp"

namespace
{
	LONG Truncate(double value) { return LONG(static_cast<__int64>(value)); }
	bool Outside(LONG x1,LONG y1,LONG x2,LONG y2,LONG left,LONG top,LONG right,LONG bottom)
	{
		return (x1<left && x2<left) || (x1>=right && x2>=right)
			|| (y1<top && y2<top) || (y1>=bottom && y2>=bottom);
	}
}
namespace GUI
{
	bool LineSystem::ClipLine(LONG& x1,LONG& y1,LONG& x2,LONG& y2) //4ADDA0.
	{
		if (!clipRectangle) return true;
		const LONG left=clipRectangle->left-offsetX,right=clipRectangle->right-offsetX;
		const LONG top=clipRectangle->top-offsetY,bottom=clipRectangle->bottom-offsetY;
		if (Outside(x1,y1,x2,y2,left,top,right,bottom)) return false;
		if (x1>=left && x1<right && x2>=left && x2<right
			&& y1>=top && y1<bottom && y2>=top && y2<bottom) return true;
		const LONG dx=x2-x1,dy=y2-y1; //Original retains the incoming deltas across all clips.
		if (x1<left) { y1+=Truncate(double(left-x1)/dx*dy); x1=left; }
		else if (x1>right-1) { y1-=Truncate((double(x1)-double(right-1))/dx*dy); x1=right-1; }
		if (x2<left) { y2+=Truncate(double(left-x2)/dx*dy); x2=left; }
		else if (x2>right-1) { y2+=Truncate((double(right-1)-double(x2))/dx*dy); x2=right-1; }
		if (y1<top) { x1+=Truncate(double(top-y1)/dy*dx); y1=top; }
		else if (y1>bottom-1) { x1-=Truncate((double(y1)-double(bottom-1))/dy*dx); y1=bottom-1; }
		if (y2<top) { x2+=Truncate(double(top-y2)/dy*dx); y2=top; }
		else if (y2>bottom-1) { x2+=Truncate((double(bottom-1)-double(y2))/dy*dx); y2=bottom-1; }
		return !Outside(x1,y1,x2,y2,left,top,right,bottom);
	}
}
