#include "PathManager.hpp"
#include <algorithm>

namespace
{
    struct LowestPathCost
    {
        bool operator()(const JE::PathNode* left, const JE::PathNode* right) const
        {
            return left->totalCost > right->totalCost;
        }
    };
} // namespace

namespace JE
{
    void ReprioritizePathNode(std::vector<PathNode*>& frontier, const PathPoint& point)
    {
        std::vector<PathNode*>::iterator entry = frontier.begin();
        const std::vector<PathNode*>::iterator end = frontier.end();
        if (entry == end)
        {
            return;
        }
        const LONG x = point.x;
        const LONG y = point.y;
        for (; entry != end; ++entry)
        {
            if ((*entry)->point.x == x && (*entry)->point.y == y)
            {
                std::push_heap(frontier.begin(), entry + 1, LowestPathCost());
                return;
            }
        }
    }

    PathNode* PopPathNode(std::vector<PathNode*>& frontier)
    {
        PathNode* node = frontier.front();
        std::pop_heap(frontier.begin(), frontier.end(), LowestPathCost());
        frontier.pop_back();
        return node;
    }

    void PushPathNode(std::vector<PathNode*>& frontier, PathNode* node)
    {
        frontier.push_back(node);
        std::push_heap(frontier.begin(), frontier.end(), LowestPathCost());
    }

    PathManager::PathManager() {}
    PathManager::~PathManager() {} // Real list destruction; original never deletes requests.
    LONG PathManager::Estimate(LONG fromX, LONG fromY, LONG toX, LONG toY)
    {
        DWORD y = static_cast<DWORD>(toY) - static_cast<DWORD>(fromY);
        DWORD x = static_cast<DWORD>(toX) - static_cast<DWORD>(fromX);
        return PathIntegerSquareRoot(static_cast<LONG>(x * x + y * y));
    }
    LONG PathManager::StepCost(const PathPoint& from, const PathPoint& to, PathRequest*)
    {
        return Estimate(from.x, from.y, to.x, to.y);
    }
    void PathManager::RequestPath(PathRequest* request)
    {
        requests.push_back(request);
    }
    BYTE PathManager::IsBlocked(BYTE flags, Entity*)
    {
        return (flags & 15) == 0;
    }
    void PathManager::Cancel(PathRequest* request)
    {
        requests.remove(request);
    }
} // namespace JE
