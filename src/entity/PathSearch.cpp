#include "PathManager.hpp"
#include "Entity.hpp"
#include "startup/MessageLoop.hpp"
#include "World.hpp"

namespace
{
    LONG AddPathCost(LONG left, LONG right)
    {
        return static_cast<LONG>(static_cast<DWORD>(left) + static_cast<DWORD>(right));
    }
    LONG PathDifference(LONG left, LONG right)
    {
        LONG difference = static_cast<LONG>(static_cast<DWORD>(left) - static_cast<DWORD>(right));
        DWORD sign = static_cast<DWORD>(difference >> 31);
        return static_cast<LONG>((static_cast<DWORD>(difference) ^ sign) - sign);
    }
} // namespace

namespace JE
{
    void PathManager::Update()
    {
        if (requests.empty())
        {
            return;
        }
        PathRequest* request = requests.front();
        if (!request)
        {
            return;
        }
        DWORD started = F3::GetElapsedTimeMS();
        if (!request->initialized)
        {
            request->nodes = new PathNode[10000];
            request->frontier = new std::vector<PathNode*>;
            request->lookup = new PathNodeLookup;
            request->initialized = 1;
            request->startedAt = F3::GetElapsedTimeMS();
            if (!InitializeRequest(request))
            {
                request->status = 2;
                Cancel(request);
                return;
            }
            if (CanTraverse(request, request->start, request->goal))
            {
                request->route = new std::list<D3DXVECTOR3>;
                request->route->push_front(request->goal);
                request->route->push_front(request->start);
                request->status = 1;
                Cancel(request);
                return;
            }
            PathNode* first = request->GetNode(request->startCell.x, request->startCell.y);
            if (!first)
            {
                request->status = 2;
                Cancel(request);
                return;
            }
            first->open = 1;
            first->closed = 0;
            first->parent = NULL;
            first->cost = 0;
            first->totalCost =
                Estimate(request->startCell.x, request->startCell.y, request->goalCell.x, request->goalCell.y);
            PushPathNode(*request->frontier, first);
        }
        while (!request->frontier->empty())
        {
            PathNode* current = PopPathNode(*request->frontier);
            // Original leaves current.open set even after removing it from the heap.
            if (PathDifference(current->point.x, request->goalCell.x) <= 3 &&
                PathDifference(current->point.y, request->goalCell.y) <= 3)
            {
                request->route = new std::list<D3DXVECTOR3>;
                request->route->push_front(request->goal);
                for (PathNode* point = current; point; point = point->parent)
                {
                    Map* map = g_World->GetMap(request->owner->mapHandle);
                    D3DXVECTOR3 position;
                    request->route->push_front(*PathCellToWorld(&position, point->point, map));
                }
                if (request->route->size() > 3)
                {
                    SmoothRoute(request);
                }
                request->status = 1;
                Cancel(request);
                return;
            }
            for (DWORD direction = 0; direction < 8; ++direction)
            {
                PathPoint neighbor;
                neighbor.x = neighbor.y = 0;
                if (!Neighbor(direction, current->point.x, current->point.y, neighbor, request))
                {
                    continue;
                }
                if (current->parent && current->parent->point.x == neighbor.x && current->parent->point.y == neighbor.y)
                {
                    continue;
                }
                LONG cost = AddPathCost(current->cost, StepCost(neighbor, current->point, request));
                LONG total =
                    AddPathCost(cost, Estimate(neighbor.x, neighbor.y, request->goalCell.x, request->goalCell.y));
                PathNode* node = request->GetNode(neighbor.x, neighbor.y);
                if (!node)
                {
                    request->status = 2;
                    Cancel(request);
                    return;
                }
                if ((node->open && total > node->totalCost) || (node->closed && total > node->totalCost))
                {
                    continue;
                }
                node->closed = 0;
                node->parent = current;
                node->cost = cost;
                node->totalCost = total;
                if (node->open)
                {
                    ReprioritizePathNode(*request->frontier, node->point);
                }
                else
                {
                    PushPathNode(*request->frontier, node);
                    node->open = 1;
                }
            }
            current->closed = 1;
            if (F3::GetElapsedTimeMS() - started > 90)
            {
                return;
            }
        }
        request->status = 2;
        Cancel(request);
    }
} // namespace JE
