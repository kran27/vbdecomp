#include "GraphicsScene.hpp"
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
	OCTREE* currentSceneOctree = NULL; //Original707F08.
}

namespace JE
{
	GraphicsSceneState::GraphicsSceneState()
		: cameraControl(standalone.cameraControl), renderer(standalone.renderer),
		maskManager(standalone.maskManager), shadowMode(standalone.shadowMode),
		startupMovies(standalone.startupMovies), sceneEnabled(standalone.sceneEnabled),
		bounds(standalone.bounds), debugBounds(standalone.debugBounds),
		currentOctree(standalone.currentOctree)
	{
		cameraControl = NULL;
		renderer = NULL;
		maskManager = NULL;
		shadowMode = 0;
		startupMovies = 1;
		sceneEnabled = 1;
		bounds = 0;
		debugBounds = 0;
		currentOctree = NULL;
	}
	GraphicsSceneState::GraphicsSceneState(::Graphics& owner)
		: cameraControl(owner.cameraControl), renderer(owner.renderer),
		maskManager(owner.maskManager), shadowMode(owner.shadowMode),
		startupMovies(owner.startupMovies), sceneEnabled(owner.sceneEnabled),
		bounds(owner.bounds), debugBounds(owner.debugBounds),
		currentOctree(owner.currentOctree)
	{
	}
	void GraphicsSceneState::ReadStartupMovieConfiguration(const Configuration::Settings& settings)
	{
		startupMovies = 1; //581C76 resets the original default before looking up the main setting.
		Configuration::Settings::const_iterator section = settings.find("Graphics");
		if (section == settings.end()) return;
		Configuration::Values::const_iterator entry = section->second.find("enable startup movies");
		if (entry != section->second.end()) startupMovies = std::atol(entry->second.c_str()) > 0;
	}
	void SetSceneOctree(OCTREE* tree) { currentSceneOctree = tree; }
	OCTREE* GetSceneOctree() { return currentSceneOctree; }

	void PrepareGraphicsScene(GraphicsSceneState& state, World& world,
		const GraphicsSceneInputs& inputs, BYTE& mapChanged)
	{
		mapChanged = 0;
		SetSceneOctree(NULL);
		if (!inputs.clientFlag10A) throw std::logic_error("Graphics582903: actual Client+10A input required");
		if (!*inputs.clientFlag10A)
		{
			if (!inputs.guiManager) throw std::logic_error("Graphics582915: actual GUI::Manager required");
			inputs.guiManager->Update();
		}
		//582936 selects the live map after GUI Update, which may change World.
		//Base548800/game46FFE0 both reach5458E0's same active-ID scan.
		Map* currentMap = world.GetMap(world.valueE0);
		if (currentMap)
		{
			OCTREE* tree = currentMap->GetOctree();
			SetSceneOctree(tree);
			if (tree)
			{
				if (!state.renderer) throw std::logic_error("Graphics58296D: actual Octree2Gfx required");
				if (tree != state.currentOctree)
				{
					mapChanged = 1;
					state.renderer->Cleanup();
					DWORD color = (DWORD(currentMap->color[0]) << 16)
						| (DWORD(currentMap->color[1]) << 8) | currentMap->color[2];
					state.renderer->Initialize(tree, 500, color, state.bounds, 1);
					state.renderer->UpdateCamera(NULL);
					currentMap->ApplyHeightFog(state.renderer);
					currentMap->ApplyFogSettings();
					if (!g_GfxSystem) throw std::logic_error("Graphics5829D8: actual Gfx_System_Interface required");
					g_GfxSystem->SetAmbientColor(&color);
					DWORD rendererColor = (DWORD(currentMap->ambient[3]) << 24)
						| (DWORD(currentMap->ambient[0]) << 16) | (DWORD(currentMap->ambient[1]) << 8)
						| currentMap->ambient[2];
					g_GfxSystem->SetRendererColor(rendererColor, NULL);
					D3DXVECTOR3 minimum = state.renderer->minimum;
					D3DXVECTOR3 maximum = state.renderer->maximum;
					g_GfxSystem->SetLevelAABB(&minimum.x, &maximum.x);
					g_GfxSystem->SetRendererOwner76(state.renderer, NULL);
					if (state.shadowMode == 1) state.renderer->floorEnabled = 1;
					state.renderer->SetInstanceFlag(currentMap->flag1D9);
					world.PrepareLightIndices(state.renderer);
					state.currentOctree = tree;
				}
				if (!state.cameraControl) throw std::logic_error("Graphics582AC2: actual camera-control interface required");
				if (state.cameraControl->Update()) state.renderer->UpdateCamera(NULL);
				else state.renderer->RestoreVisibility();
				state.renderer->ClearHiddenMaterials();
				std::set<int> hiddenMaterials;
				currentMap->CopyHiddenMaterials(hiddenMaterials);
				state.renderer->SetHiddenMaterials(hiddenMaterials);
				if (state.sceneEnabled)
				{
					if (!inputs.clientFlag108) throw std::logic_error("Graphics582B43: actual Client+108 input required");
					if (!*inputs.clientFlag108) state.renderer->SubmitVisible(state.debugBounds, NULL);
				}
			}
		}
		if (!state.maskManager) throw std::logic_error("Graphics582B98: actual MapMaskManager required");
		state.maskManager->Update();
	}
}
