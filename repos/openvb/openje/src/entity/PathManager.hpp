#pragma once

#include <windows.h>
#include <d3dx8math.h>
#include <list>
#include <vector>
#include "MapGrid.hpp"

//The supplied VC7 library supplies the actual hash container. Current modern
//MSVC removed hash_map; its host lane uses the real standard hash container.
#include <hash_map>

namespace JE
{
	class Entity;
	class Map;
	struct PathPoint { LONG x, y; };
	LONG PathIntegerSquareRoot(LONG value); //5601F0, integer square-root rounding.
	PathPoint* WorldToPathCell(PathPoint* output, const D3DXVECTOR3& position, Map* map); //560680, same output pointer.
	D3DXVECTOR3* PathCellToWorld(D3DXVECTOR3* output, const PathPoint& point, Map* map); //560710, Y0; same output pointer.

	struct PathNode
	{
		PathNode(); //560490; only the two point coordinates are initialized.
		PathPoint point;
		PathNode* parent;
		LONG cost, totalCost;
		BYTE open, closed;
	};
	struct PathPointHash
	{
		enum { bucket_size = 4, min_buckets = 8 };
		size_t operator()(const PathPoint& point) const;
		bool operator()(const PathPoint& left, const PathPoint& right) const;
	};
	struct PathPointHashEqual
	{
		bool operator()(const PathPoint& left, const PathPoint& right) const
		{
			return PathPointHash()(left) == PathPointHash()(right);
		}
	};
	typedef std::hash_multimap<PathPoint, PathNode*, PathPointHash> PathNodeLookup;
	struct PathRequest
	{
		PathRequest(Entity* owner, D3DXVECTOR3 destination, BYTE adjustGoal,
			DWORD value58, BYTE flag39); //560390, actual96-byte request value.
		~PathRequest(); //562E90, cancels pending request, then destroys owned work.
		PathNode* GetNode(LONG x, LONG y); //562F90/560460, fixed10000-node limit.
		DWORD status; //0 pending,1 complete,2 failed.
		std::list<D3DXVECTOR3>* route; //+04, owned, points stored start->goal.
		DWORD startedAt; //+08, unwritten until initialization.
		Entity* owner; //+0C, borrowed.
		PathPoint startCell, goalCell; //+10/+18.
		D3DXVECTOR3 start, goal; //+20/+2C.
		BYTE initialized, flag39; //+38/+39; following padding stays unwritten.
		PathNode* nodes; //+3C, owning10000-element workspace.
		DWORD usedNodes; //+40.
		std::vector<PathNode*>* frontier; //+44, owning container, borrowed node pointers.
		PathNodeLookup* lookup; //+48, owning official hash container.
		MapGrid* terrain; EntityGrid* occupancy; MapGrid* alternateTerrain; //+4C/+50/+54, borrowed.
		DWORD value58;
		BYTE adjustGoal; //+5C.
	private:
		PathNode* AcquireNode(); //560460, next slot in the fixed10000-node workspace.
		PathRequest(const PathRequest&);
		PathRequest& operator=(const PathRequest&);
	};
	class PathManager
	{
	public:
		PathManager(); // 0x560B80; actual list of borrowed request pointers.
		~PathManager(); // 0x560BB0, nonvirtual in original; list nodes only.
		virtual LONG Estimate(LONG fromX, LONG fromY, LONG toX, LONG toY); // 0x5604A0.
		virtual LONG StepCost(const PathPoint& from, const PathPoint& to, PathRequest* request); // 0x5604D0.
		virtual BYTE CanTraverse(PathRequest* request, D3DXVECTOR3 from, D3DXVECTOR3 to); // 0x562600.
		virtual BYTE IsBlocked(BYTE flags, Entity* entity); // 0x560480.
		void RequestPath(PathRequest* request); //561B90, append borrowed request.
		void Cancel(PathRequest* request); //560B10, remove all matching pointers.
		void Update(); //563090, first-request A* with90ms yield.
		std::list<PathRequest*> requests; // Original+4, nodes owned; requests borrowed.
	private:
		BYTE InitializeRequest(PathRequest* request); //5628E0.
		BYTE Neighbor(DWORD direction, LONG x, LONG y, PathPoint& output, PathRequest* request); //560C00.
		void SmoothRoute(PathRequest* request); //561FA0.
		PathManager(const PathManager&);
		PathManager& operator=(const PathManager&);
	};
}
