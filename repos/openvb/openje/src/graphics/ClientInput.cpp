#include "ClientInput.hpp"
#include "ClientDebugReport.hpp"
#include "gui/GraphicsInput.hpp"
#include "CameraControl.hpp"
#include "entity/Entity.hpp"
#include "network/CommandProcessor.hpp"
#include "startup/ConfigurationObject.hpp"
#include <cstring>
#include <stdexcept>

namespace JE
{
	namespace
	{
		const float DegreesPerRadian = 57.295776f;
		LONG BindingKey(DWORD scan, DWORD subtype, DWORD modifiers)
		{ return (scan & 0xFFFF) | ((DWORD(subtype & 15) | ((modifiers & 15) << 4)) << 16); }
		struct OwnedEntitySearch
		{
			DWORD clientID, selectedID;
			Entity* found;
			BYTE stopAtSelected;
		};
		void FindOwnedEntity(Entity* entity, void* context)
		{
			OwnedEntitySearch& search = *static_cast<OwnedEntitySearch*>(context);
			if (search.stopAtSelected == 2 || !entity->visibilityOwnerFlag ||
				entity->visibilityOwnerFlag != search.clientID) return;
			search.found = entity;
			if (!search.stopAtSelected || entity->entityID == search.selectedID)
				search.stopAtSelected = 2;
		}
		void DragCamera(ClientInputState& state, CameraControl& camera, LONG dx, LONG dy,
			ClientInputServices& services) //589130.
		{
			if (!state.enabled || state.mouseRotate) return;
			D3DXVECTOR3 target;
			camera.GetTarget(target);
			D3DXMATRIX result, toOrigin, back, rotation;
			D3DXMatrixTranslation(&result, target.x - dx, target.y, target.z + dy);
			D3DXMatrixTranslation(&toOrigin, -target.x, -target.y, -target.z);
			D3DXMatrixTranslation(&back, target.x, target.y, target.z);
			const float angle = static_cast<float>(camera.orbit.azimuth * DegreesPerRadian * 0.017453292f);
			D3DXMatrixRotationY(&rotation, angle);
			D3DXMatrixMultiply(&result, &result, &toOrigin);
			D3DXMatrixMultiply(&result, &result, &rotation);
			D3DXMatrixMultiply(&result, &result, &back);
			const double delay = services.FieldOfViewRadians() * double(DegreesPerRadian) - 6.0;
			if (state.dragDelay < delay)
			{
				//Original __ftol2, not rounding-to-nearest.
				state.dragDelay = static_cast<LONG>(delay);
				camera.SetTarget(D3DXVECTOR3(result._41, result._42, result._43));
			}
			else state.dragDelay = static_cast<LONG>(DWORD(state.dragDelay) - 1);
		}
	}

	ClientInputState::ClientInputState(DWORD clock)
		: mouseOverGUI(0), mouseX(0), mouseY(0), wheel(0), left(0), middle(0), right(0),
		mouseRotate(0), lastPickingTime(clock), hoveredEntityID(0), selectedEntityID(0),
		valueE4(0), groundPosition(0,0,0), selectionEffect(NULL), hoverEffect(NULL),
		clientID(0), enabled(0), keyboardScroll(0), dragDelay(0)
	{
		//Original58E900 does not write+E8, retained as groundHit.
	}
	BOOL ClientInputServices::CursorPosition(POINT* output) { return ::GetCursorPos(output); }
	void ClientInputServices::PickMouse(ClientInputState&, BYTE)
	{ throw std::logic_error("unbound Client mouse picking58AB80: Graphics/World/clock required"); }
	void ClientInputServices::SelectOwnedEntity(Entity*)
	{ throw std::logic_error("unbound Actionbar numeric-ID refresh41C9D0 after Client selection publication"); }
	void ClientInputServices::SendWorldAction(DWORD, DWORD, const D3DXVECTOR3&)
	{ throw std::logic_error("unbound Client world-action message589FD0/58A0E0 ->59C4F0"); }
	void ClientInputServices::DebugPickPosition(LONG, LONG)
	{ throw std::logic_error("unrecovered GameClient populated debug-pick4617DA"); }

	ClientHotKeys::ClientHotKeys(Input::Win32Input& owner)
		: input(owner), console(NULL) {} //Explicit map-only adapter; native lifetime still saves fixed settings.
	ClientHotKeys::ClientHotKeys(Input::Win32Input& owner, ConsoleCommandProcessor* processor)
		: input(owner), console(processor) {}
	void ClientHotKeys::Initialize() //5998C0.
	{
		const Configuration::Settings& settings = *F3::GetSettingsObject();
		Configuration::Settings::const_iterator section = settings.find("HotKeys");
		if (section == settings.end()) return;
		for (Configuration::Values::const_iterator it = section->second.begin(); it != section->second.end(); ++it)
		{
			const char* name = it->first.c_str();
			BYTE subtype = 0;
			DWORD modifiers = 0;
			if (it->first.size() > 1 && (*name == '-' || *name == '+'))
			{ subtype = *name == '-' ? 1 : 0; ++name; }
			for (int pass = 0; pass != 2; ++pass)
			{
				if (!std::strncmp(name, "Ctrl+", 5)) { modifiers |= 1; name += 5; }
				if (!std::strncmp(name, "Shift+", 6)) { modifiers |= 2; name += 6; }
			}
			const DWORD scan = FindKey(name);
			if (scan) Bind(scan, subtype, modifiers, it->second.c_str());
		}
	}
	BYTE ClientHotKeys::Bind(DWORD scan, DWORD subtype, DWORD modifiers, const char* command)
	{
		bindings[BindingKey(scan, subtype, modifiers)] = command;
		return 1;
	}
	void ClientHotKeys::Unbind(DWORD scan, DWORD subtype, DWORD modifiers)
	{
		std::map<LONG,std::string>::iterator found = bindings.find(BindingKey(scan, subtype, modifiers));
		if (found != bindings.end())
			bindings.erase(found);
	}
	BYTE ClientHotKeys::FindBinding(DWORD& scan, DWORD& subtype, DWORD& modifiers,
		const char* command) const
	{
		for (std::map<LONG,std::string>::const_iterator i = bindings.begin();
			i != bindings.end(); ++i)
		{
			if (!_stricmp(command, i->second.c_str()))
			{
				scan = i->first & 0xFFFF;
				subtype = (i->first >> 16) & 15;
				modifiers = (i->first >> 20) & 15;
				return 1;
			}
		}
		return 0;
	}
	DWORD ClientHotKeys::FindKey(const char* name) const
	{
		return input.FindKey(name);
	}
	BYTE ClientHotKeys::Handle(const Input::InputEvent& event, ClientInputServices& services) const
	{
		if (event.category) return 0;
		const LONG key = (DWORD(event.x) & 0xFFFF) |
			((event.subtype & 15 | ((event.modifiers & 15) << 4)) << 16); //599748; omitted by decompiler.
		std::map<LONG,std::string>::const_iterator found = bindings.find(key);
		if (found == bindings.end()) return 0;
		if (console) console->Queue(found->second.c_str()); //580D10; dispatch occurs later587FD5.
		else services.ExecuteCommand(found->second.c_str()); //Explicit older map-factor service.
		return 1;
	}
	BYTE ClientHotKeys::Handle(const Input::InputEvent& event) const //599720.
	{
		if (event.category) return 0;
		const LONG key = (DWORD(event.x) & 0xFFFF) |
			((event.subtype & 15 | ((event.modifiers & 15) << 4)) << 16);
		std::map<LONG,std::string>::const_iterator found = bindings.find(key);
		if (found == bindings.end()) return 0;
		console->Queue(found->second.c_str()); //Captured original+8 owner; required, not an empty fallback.
		return 1;
	}
	void ClientHotKeys::Load(const Configuration::Settings& settings)
	{
		Configuration::Settings::const_iterator section = settings.find("HotKeys");
		if (section == settings.end()) return;
		for (Configuration::Values::const_iterator it = section->second.begin(); it != section->second.end(); ++it)
		{
			const char* name = it->first.c_str();
			BYTE subtype = 0;
			DWORD modifiers = 0;
			if (it->first.size() > 1 && (*name == '-' || *name == '+'))
			{ subtype = *name == '-' ? 1 : 0; ++name; }
			for (int pass = 0; pass != 2; ++pass)
			{
				if (!std::strncmp(name, "Ctrl+", 5)) { modifiers |= 1; name += 5; }
				if (!std::strncmp(name, "Shift+", 6)) { modifiers |= 2; name += 6; }
			}
			const DWORD scan = FindKey(name);
			if (scan) Bind(scan, subtype, modifiers, it->second.c_str());
		}
	}

	BYTE HandleGameMouseInput(ClientInputState& state, const Input::InputEvent& event,
		ClientInputServices& services)
	{
		if (event.subtype)
		{
			if (event.subtype != 2 || event.button) return 0;
			services.PickMouse(state, 1);
			if (state.hoveredEntityID)
			{
				Entity* hovered = GetEntityPtr(state.hoveredEntityID);
				if (hovered && hovered->visibilityOwnerFlag == state.clientID)
				{
					state.selectedEntityID = state.hoveredEntityID;
					services.SelectOwnedEntity(hovered);
				}
				return 1;
			}
			if (event.modifiers & 1)
			{
				if (!state.selectedEntityID)
				{
					OwnedEntitySearch search = {state.clientID,0,NULL,0};
					VisitEntities(FindOwnedEntity, &search);
					if (search.found) state.selectedEntityID = search.found->entityID;
				}
				if (state.selectedEntityID) services.SendWorldAction(59, state.selectedEntityID, state.groundPosition);
				return 0; //46175B reaches the initial zero result, even after sending59.
			}
			else if (state.selectedEntityID && state.groundHit)
				services.SendWorldAction(60, state.selectedEntityID, state.groundPosition);
			return 1;
		}
		if (services.HasDebugWindow() && (event.modifiers & 2)) services.DebugPickPosition(event.x,event.y);
		return 0;
	}
	BYTE HandleGameCursorInput(ClientInputState& state, const Input::InputEvent& event,
		ClientInputServices& services)
	{
		if (!state.enabled || event.category != 2) return 0;
		//Both down-button entries call actual436230, which returns0 unconditionally.
		if (event.subtype != 2) return 0;
		if (!event.button)
			throw std::logic_error("unrecovered GameCursorManager left-release436B40/436AD0");
		if (event.button != 2) return 0;
		if (event.modifiers & 1)
			return F3::DebugPrintEntityInfo(event.x,event.y);
		Entity* selected = GetEntityPtr(state.selectedEntityID);
		Entity* hovered = GetEntityPtr(state.hoveredEntityID);
		if (!selected || !hovered) return 0;
		throw std::logic_error("unrecovered GameCursorManager contextual right-release436FC0/587A80");
	}
	BYTE HandleClientCameraInput(ClientInputState& state, CameraControl& camera,
		const Input::InputEvent& event, ClientInputServices& services)
	{
		if (!state.enabled || event.category != 2) return 0;
		if (!event.subtype)
		{
			if (state.right == 1) DragCamera(state,camera,event.deltaX,event.deltaY,services);
			if (state.mouseRotate)
			{
				if (event.deltaX)
				{
					const double step = camera.azimuthSpeed * double(0.00039999999f) * 33.0f;
					camera.SetAzimuth(static_cast<float>(camera.orbit.azimuth * double(DegreesPerRadian) +
						(event.deltaX > 0 ? step : -step)));
				}
				if (event.deltaY)
				{
					const double step = camera.elevationSpeed * double(0.00039999999f) * 33.0f;
					camera.SetElevation(static_cast<float>(camera.orbit.elevation * double(DegreesPerRadian) +
						(event.deltaY > 0 ? -step : step)));
				}
				return 1;
			}
		}
		if (!state.mouseOverGUI && event.subtype == 4)
		{
			const float step = static_cast<float>(camera.fieldOfViewSpeed * double(0.0024999999f) * 33.0f);
			camera.SetFieldOfView(static_cast<float>(services.FieldOfViewRadians() * double(DegreesPerRadian) +
				(event.wheel < 0 ? step : -step)));
		}
		return 0;
	}
	void UpdateClientCursor(ClientInputState& state, CameraControl& camera, ClientInputServices& services)
	{
		if (!state.enabled || state.mouseRotate) return;
		POINT cursor;
		if (!services.CursorPosition(&cursor)) return;
		const WORD retained = camera.movementFlags & 0xFFF0;
		LONG width, height;
		if (services.Windowed()) { width = GetSystemMetrics(SM_CXSCREEN); height = GetSystemMetrics(SM_CYSCREEN); }
		else services.ScreenDimensions(width,height);
		LONG borderWidth, borderHeight;
		services.ScreenDimensions(borderWidth,borderHeight);
		borderWidth = static_cast<LONG>(borderWidth * double(camera.scrollBorder));
		borderHeight = static_cast<LONG>(borderHeight * double(camera.scrollBorder));
		WORD flags = 0;
		if (cursor.x >= 0 && cursor.x < borderWidth) flags = 1;
		if (cursor.x >= width-borderWidth && cursor.x < width) flags |= 2;
		if (cursor.y >= 0 && cursor.y < borderHeight) flags |= 4;
		if (cursor.y >= height-borderHeight && cursor.y < height) flags |= 8;
		camera.movementFlags = retained | (flags ? flags : state.keyboardScroll);
	}
	void ProcessClientInput(ClientInputState& state, Input::Win32Input& input,
		CameraControl& camera, ClientHotKeys& hotKeys, GUI::GraphicsInputServices& graphics,
		ClientInputServices& services)
	{
		Input::InputEvent event;
		while (input.PopEvent(event))
		{
			if (event.category == 2)
			{
				state.mouseX = event.x; state.mouseY = event.y; state.wheel = event.wheel;
				state.left = event.left; state.middle = event.middle; state.right = event.right;
				state.mouseOverGUI = 1;
			}
			if (!GUI::DispatchGraphicsInput(event,graphics))
			{
				if (event.category == 2) state.mouseOverGUI = 0;
				if (!services.SecondaryInput(event) &&
					!(event.category == 2 && HandleGameMouseInput(state,event,services)) &&
					!HandleClientCameraInput(state,camera,event,services) && state.enabled)
					hotKeys.Handle(event,services);
			}
		}
		if (!services.HasGUICapture()) UpdateClientCursor(state,camera,services);
	}
	BYTE ExecuteClientCameraCommand(const char* command, ClientInputState& state, CameraControl& camera)
	{
		if (!_stricmp(command,"CameraScrollLeft_On")) state.keyboardScroll |= 1;
		else if (!_stricmp(command,"CameraScrollRight_On")) state.keyboardScroll |= 2;
		else if (!_stricmp(command,"CameraScrollUp_On")) state.keyboardScroll |= 4;
		else if (!_stricmp(command,"CameraScrollDown_On")) state.keyboardScroll |= 8;
		else if (!_stricmp(command,"CameraScrollLeft_Off")) state.keyboardScroll &= ~1;
		else if (!_stricmp(command,"CameraScrollRight_Off")) state.keyboardScroll &= ~2;
		else if (!_stricmp(command,"CameraScrollUp_Off")) state.keyboardScroll &= ~4;
		else if (!_stricmp(command,"CameraScrollDown_Off")) state.keyboardScroll &= ~8;
		else if (!_stricmp(command,"CameraZoomIn_On")) camera.movementFlags |= 0x10;
		else if (!_stricmp(command,"CameraZoomIn_Off")) camera.movementFlags &= ~0x10;
		else if (!_stricmp(command,"CameraZoomOut_On")) camera.movementFlags |= 0x20;
		else if (!_stricmp(command,"CameraZoomOut_Off")) camera.movementFlags &= ~0x20;
		else if (!_stricmp(command,"CameraRotateLeft")) camera.movementFlags ^= 0x40;
		else if (!_stricmp(command,"CameraRotateRight")) camera.movementFlags ^= 0x80;
		else if (!_stricmp(command,"CameraTiltUp")) camera.movementFlags ^= 0x100;
		else if (!_stricmp(command,"CameraTiltDown")) camera.movementFlags ^= 0x200;
		else if (!_stricmp(command,"CameraMouseRotate_On")) state.mouseRotate = 1;
		else if (!_stricmp(command,"CameraMouseRotate_Off")) state.mouseRotate = 0;
		else if (!_stricmp(command,"CameraToggleConstraints")) camera.SetConstraints(camera.constraints == 0);
		else if (!_stricmp(command,"CameraAzimuthInc"))
			camera.SetTargetAzimuth(static_cast<float>((camera.orbit.azimuth * double(DegreesPerRadian) * 0.022222223f + 1.0) * 45.0));
		else if (!_stricmp(command,"CameraAzimuthDec"))
			camera.SetTargetAzimuth(static_cast<float>(camera.orbit.azimuth * double(DegreesPerRadian) * 0.022222223f * 45.0 - 45.0));
		else if (!_stricmp(command,"CameraTopDown")) camera.ToggleTopDown(1);
		else if (!_stricmp(command,"CameraTop")) camera.Top(1);
		else if (!_stricmp(command,"CameraDown")) camera.Down(1);
		else if (!_stricmp(command,"CameraReset"))
		{
			camera.Reset();
			OwnedEntitySearch search = {state.clientID,state.selectedEntityID,NULL,1};
			VisitEntities(FindOwnedEntity,&search);
			if (search.found) camera.SetTarget(search.found->position);
		}
		else return 0;
		return 1;
	}
}
