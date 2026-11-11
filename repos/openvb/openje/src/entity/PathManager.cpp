#include "PathManager.hpp"

namespace JE
{
	PathManager::PathManager() {}
	PathManager::~PathManager() {} // Real list destruction; original never deletes requests.
	LONG PathManager::Estimate(LONG fromX, LONG fromY, LONG toX, LONG toY)
	{
		DWORD y = static_cast<DWORD>(toY) - static_cast<DWORD>(fromY);
		DWORD x = static_cast<DWORD>(toX) - static_cast<DWORD>(fromX);
		return PathIntegerSquareRoot(static_cast<LONG>(x*x + y*y));
	}
	LONG PathManager::StepCost(const PathPoint& from, const PathPoint& to, PathRequest*)
	{
		return Estimate(from.x, from.y, to.x, to.y);
	}
	void PathManager::RequestPath(PathRequest* request)
	{
		requests.push_back(request);
	}
	BYTE PathManager::IsBlocked(BYTE flags, Entity*) { return (flags & 15) == 0; }
	void PathManager::Cancel(PathRequest* request)
	{
		requests.remove(request);
	}
}
