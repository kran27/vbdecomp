#include "GraphicsScene.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "Graphics.hpp"
#include "CameraController.hpp"
#include "MapMaskManager.hpp"
#include "OctreeRender.hpp"
#include "SystemInterface.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "gui/Manager.hpp"
#include <set>
#include <cstdlib>
#include <stdexcept>

namespace
{
    OCTREE* currentSceneOctree = NULL; // Original707F08.
}

namespace JE
{
    GraphicsSceneState::GraphicsSceneState(::Graphics& owner)
        : cameraControl(owner.cameraControl), renderer(owner.renderer), maskManager(owner.maskManager),
          shadowMode(owner.shadowMode), startupMovies(owner.startupMovies), sceneEnabled(owner.sceneEnabled),
          bounds(owner.bounds), debugBounds(owner.debugBounds), currentOctree(owner.currentOctree)
    {
    }
    void SetSceneOctree(OCTREE* tree)
    {
        currentSceneOctree = tree;
    }
    OCTREE* GetSceneOctree()
    {
        return currentSceneOctree;
    }

    void DetachOctreeConsumers(OCTREE* tree)
    {
        if (!tree)
        {
            return;
        }

        // VFX and weather use this borrowed global octree, even outside the renderer.
        if (currentSceneOctree == tree)
        {
            currentSceneOctree = NULL;
        }

        // Only the currently connected client can own this renderer.
        F3::NetworkComponent* network = F3::g_pNetworkInstance;
        if (!network || !network->client)
        {
            return;
        }

        JE::Client* client = dynamic_cast<JE::Client*>(network->client);
        if (client && client->child0x74)
        {
            client->child0x74->DetachOctree(tree);
        }
    }
} // namespace JE
