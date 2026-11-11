#include <winsock2.h>
#include "Graphics.hpp"
#include "startup/NetworkComponent.hpp"
#include "startup/MessageLoop.hpp"
#include "CameraController.hpp"
#include "GraphicsScene.hpp"
#include "MapMaskManager.hpp"
#include "OctreeRender.hpp"
#include "SystemInterface.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "gui/Manager.hpp"
#include "gui/Win32Input.hpp"
#include "network/EngineClient.hpp"
#include <set>

OCTREE_Camera* Graphics::GetPickingCamera() //581190.
{
	OCTREE_Camera* result = NULL;
	if (renderer) result = renderer->camera;
	return result;
}

void Graphics::PrepareScene(BYTE* mapChanged)
{
	*mapChanged = 0;
	JE::SetSceneOctree(NULL);
	if (!JE::EngineClientServices::CurrentClient()->field0x10A) ::GUI::g_Manager.Update();
	//58291A reloads the published World only after GUI Update has returned.
	JE::World* world = JE::g_World;
	JE::Map* map = world->GetMap(world->valueE0);
	if (map)
	{
		OCTREE* tree = map->GetOctree();
		JE::SetSceneOctree(tree);
		if (tree)
		{
			if (tree != currentOctree)
			{
				*mapChanged = 1;
				renderer->Cleanup();
				DWORD color = (DWORD(map->color[0]) << 16)
					| (DWORD(map->color[1]) << 8) | DWORD(map->color[2]);
				renderer->Initialize(tree, 500, color, bounds, 1);
				renderer->UpdateCamera(NULL);
				map->ApplyHeightFog(renderer);
				map->ApplyFogSettings();
				g_GfxSystem->SetAmbientColor(&color);
				const DWORD rendererColor = (DWORD(map->ambient[3]) << 24)
					| (DWORD(map->ambient[0]) << 16) | (DWORD(map->ambient[1]) << 8)
					| DWORD(map->ambient[2]);
				g_GfxSystem->SetRendererColor(rendererColor, NULL);
				Octree2Gfx* selectedRenderer = renderer;
				const D3DXVECTOR3 minimum = selectedRenderer->minimum;
				const D3DXVECTOR3 maximum = selectedRenderer->maximum;
				g_GfxSystem->SetLevelAABB(&minimum.x, &maximum.x);
				g_GfxSystem->SetRendererOwner76(renderer, NULL);
				if (shadowMode == 1) renderer->floorEnabled = 1;
				renderer->SetInstanceFlag(map->flag1D9);
				//582AA5 reloads World after the backend/renderer callbacks.
				JE::g_World->PrepareLightIndices(renderer);
				currentOctree = tree;
			}
			if (cameraControl->Update()) renderer->UpdateCamera(NULL);
			else renderer->RestoreVisibility();
			renderer->ClearHiddenMaterials();
			std::set<int> hiddenMaterials;
			map->CopyHiddenMaterials(hiddenMaterials);
			renderer->SetHiddenMaterials(hiddenMaterials);
			if (sceneEnabled && !JE::EngineClientServices::CurrentClient()->field0x108)
				renderer->SubmitVisible(debugBounds, NULL);
		}
	}
	maskManager->Update();
}

void Graphics::Present()
{
	if (!static_cast<JE::Client*>(F3::g_pNetworkInstance->client)->field0x10A) ::GUI::g_Manager.Render();
	const DWORD now = F3::GetElapsedTimeMS();
	backend->AdvanceTime(now - previousClock);
	//5811F6 reloads this backend after the AdvanceTime callback.
	backend->RenderFrame();
	previousClock = now;
}

void Graphics::AttachCamera(Gfx_CamCtrl* control)
{
	cameraControl = control;
	backend->SetCameraController(control);
}

BYTE Graphics::ProcessInput(const JE::Input::InputEvent& event)
{
	BYTE result = 0;
	switch (event.category)
	{
	case 0:
		switch (event.subtype)
		{
		case 0:
			result = ::GUI::g_Manager.ProcessKey(event.x, 1, event.modifiers);
			break;
		case 1:
			result = ::GUI::g_Manager.ProcessKey(event.x, 0, event.modifiers);
			break;
		}
		break;
	case 1:
		result = ::GUI::g_Manager.ProcessCharacter(char(event.character), event.modifiers);
		break;
	case 2:
		::GUI::MouseState mouse;
		mouse.x = event.x;
		mouse.y = event.y;
		mouse.wheel = event.wheel;
		mouse.buttons = 0;
		if (event.left) mouse.buttons = 1;
		if (event.middle) mouse.buttons |= 2;
		if (event.right) mouse.buttons |= 4;
		if (event.subtype == 3) mouse.buttons <<= 16;
		result = ::GUI::g_Manager.ProcessMouse(&mouse, event.modifiers);
		break;
	}
	return result;
}
