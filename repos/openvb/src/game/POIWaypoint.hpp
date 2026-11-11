#pragma once

#include "gui/Label.hpp"
#include <cstddef>

namespace GUI { class LineSystem; }

class POIWaypoint : public GUI::Label
{
public:
	POIWaypoint(); //4587E0.
	virtual ~POIWaypoint(); //4588C0.
	virtual void Refresh(); //458930.
	virtual void Draw(); //458950.
	virtual float SetDepth(float depth, float increment); //458980.

	LONG coordinateX, coordinateY; //E64/E68.
	float valueE6C, valueE70, valueE74, valueE78; //Float route values; see458BC0/459680.
	GUI::LineSystem* lineSystem; //E7C, owned; published after construction.
private:
	POIWaypoint(const POIWaypoint&);
	POIWaypoint& operator=(const POIWaypoint&);
};

#if defined(_MSC_VER) && _MSC_VER == 1300
typedef char POIWaypointOriginalSize[(sizeof(POIWaypoint) == 0xE80) ? 1 : -1];
typedef char POIWaypointOriginalX[(offsetof(POIWaypoint, coordinateX) == 0xE64) ? 1 : -1];
typedef char POIWaypointOriginalY[(offsetof(POIWaypoint, coordinateY) == 0xE68) ? 1 : -1];
typedef char POIWaypointOriginalE6C[(offsetof(POIWaypoint, valueE6C) == 0xE6C) ? 1 : -1];
typedef char POIWaypointOriginalE70[(offsetof(POIWaypoint, valueE70) == 0xE70) ? 1 : -1];
typedef char POIWaypointOriginalE74[(offsetof(POIWaypoint, valueE74) == 0xE74) ? 1 : -1];
typedef char POIWaypointOriginalE78[(offsetof(POIWaypoint, valueE78) == 0xE78) ? 1 : -1];
typedef char POIWaypointOriginalLines[(offsetof(POIWaypoint, lineSystem) == 0xE7C) ? 1 : -1];
#endif
