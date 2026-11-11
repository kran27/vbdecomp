#include "PathManager.hpp"

namespace
{
	void __stdcall AppendCurve(D3DXVECTOR3 a, D3DXVECTOR3 b, D3DXVECTOR3 c,
		D3DXVECTOR3 d, std::list<D3DXVECTOR3>& output) //561BD0, four values+pointer, RET52.
	{
		//Actual authored quarter weights. The third sample's0.22656301 is
		//deliberately retained rather than replacing it with SDK Catmull-Rom.
		const float coefficients[3][4] = {
			{-0.0703125f,0.8671875f,0.2265625f,-0.0234375f},
			{-0.0625f,0.5625f,0.5625f,-0.0625f},
			{-0.0234375f,0.22656301f,0.8671875f,-0.0703125f}};
		for (int sample = 0; sample < 3; ++sample)
		{
			D3DXVECTOR3 point;
			for (int axis = 0; axis < 3; ++axis)
			{
				float first = a[axis] * coefficients[sample][0];
				float second = b[axis] * coefficients[sample][1];
				float third = c[axis] * coefficients[sample][2];
				float fourth = d[axis] * coefficients[sample][3];
				float pair = first + second;
				float triple = pair + third;
				point[axis] = triple + fourth;
			}
			output.push_back(point);
		}
	}
	bool EqualPoint(const D3DXVECTOR3& left, const D3DXVECTOR3& right)
	{
		return left.x == right.x && left.y == right.y && left.z == right.z;
	}
}

namespace JE
{
	void PathManager::SmoothRoute(PathRequest* request)
	{
		typedef std::list<D3DXVECTOR3>::iterator RouteIterator;
		RouteIterator anchor = request->route->begin();
		RouteIterator current = anchor; ++current;
		RouteIterator last = request->route->end(); --last;
		while (current != last)
		{
			RouteIterator next = current; ++next;
			const D3DXVECTOR3 to = *next;
			const D3DXVECTOR3 from = *anchor;
			if (CanTraverse(request, from, to))
			{
				//562027 reloads the owner after the callback, while current
				//continues to refer to its held node. Advance before erasing.
				std::list<D3DXVECTOR3>& route = *request->route;
				RouteIterator erased = current;
				const bool sentinel = erased == route.end();
				++current;
				if (!sentinel) route.erase(erased);
			}
			else
			{
				anchor = current;
				++current; //Reload the held node's next link after CanTraverse.
			}
		}
		if (request->route->size() <= 3) return;
		std::list<D3DXVECTOR3>* smoothed = new std::list<D3DXVECTOR3>;
		RouteIterator first = request->route->begin();
		smoothed->push_back(*first);
		//5620E4 onward copies the initial curve only after the first append.
		RouteIterator second = first; ++second;
		RouteIterator third = second; ++third;
		AppendCurve(*first, *first, *second, *third, *smoothed);

		//562174 reloads the route after that curve. Hold its penultimate node
		//and the preceding terminal-window anchor while walking the live list.
		RouteIterator terminal = request->route->end(); --terminal; --terminal;
		RouteIterator finalWindow = terminal; --finalWindow;
		RouteIterator point = request->route->begin();
		while (point != terminal)
		{
			RouteIterator next = point; ++next;
			RouteIterator following = next; ++following;
			const D3DXVECTOR3 a = *point;
			const D3DXVECTOR3 b = *next;
			const D3DXVECTOR3 c = *following;
			D3DXVECTOR3 d;
			if (point == finalWindow) d = c;
			else { RouteIterator fourth = following; ++fourth; d = *fourth; }
			//All four control values are captured before output allocation.
			smoothed->push_back(b);
			AppendCurve(a, b, c, d, *smoothed);
			if (point == finalWindow) smoothed->push_back(c);
			++point;
		}
		request->route->clear(); //56233C reloads the current owner; its object leaks.
		for (std::list<D3DXVECTOR3>::iterator point = smoothed->begin(); point != smoothed->end(); ++point)
		{
			std::list<D3DXVECTOR3>::iterator following = point; ++following;
			while (following != smoothed->end())
			{
				if (EqualPoint(*point, *following)) following = smoothed->erase(following);
				else ++following;
			}
		}
		//561FA0 clears the previous list but replaces its pointer without deleting
		//that list object/head. Preserve the original normal-path leak.
		request->route = smoothed;
	}
}
