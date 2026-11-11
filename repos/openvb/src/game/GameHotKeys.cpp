#include "game/GameHotKeys.hpp"

namespace F3
{
	GameHotKeys::~GameHotKeys()
	{
		//45F2D0 delegates to599CD0; the base saves once before its map dies.
	}

	void GameHotKeys::LoadGameDefaults() //468AE0, actual ordered registrations.
	{
		Bind(input.FindKey("Esc"), 0, 0, "EscapeMenu");
		Bind(input.FindKey("A"), 0, 1, "AttackModeCycle");
		Bind(input.FindKey("A"), 0, 0, "AttackModeSelect");
		Bind(input.FindKey("C"), 0, 0, "CharacterRecordWindow");
		Bind(input.FindKey("F"), 0, 0, "FeedbackWindow");
		Bind(input.FindKey("G"), 0, 0, "GameOptions");
		Bind(input.FindKey("H"), 0, 0, "HotKeyOptions");
		Bind(input.FindKey("I"), 0, 0, "InventoryWindow");
		Bind(input.FindKey("L"), 0, 0, "LoadGameOptions");
		Bind(input.FindKey("M"), 0, 0, "MultiplayerOptions");
		Bind(input.FindKey("P"), 0, 0, "PipBoy");
		Bind(input.FindKey("S"), 0, 0, "SneakToggle");
		Bind(input.FindKey("U"), 0, 0, "UseSkill");
		Bind(input.FindKey("V"), 0, 0, "VideoOptions");
		Bind(input.FindKey("T"), 0, 0, "WeaponLinkToggle");
		Bind(input.FindKey("W"), 0, 0, "WeaponToggle");
		Bind(input.FindKey("2"), 0, 0, "ActionbarWindow");
		Bind(input.FindKey("`"), 0, 0, "ConsoleWindow");
		Bind(input.FindKey("B"), 0, 1, "ToggleBilinearFiltering");
		Bind(input.FindKey("F"), 0, 1, "ToggleFullscreen");
		Bind(input.FindKey("F1"), 0, 0, "ToggleStatistics");
		Bind(input.FindKey("K"), 0, 1, "Kill");
		Bind(input.FindKey("L"), 0, 1, "ToggleLighting");
		Bind(input.FindKey("N"), 0, 1, "RequestNewGame");
		Bind(input.FindKey("O"), 0, 1, "ToggleOctreeDrawing");
		Bind(input.FindKey("M"), 0, 1, "MemoryPoolTest");
		Bind(input.FindKey("S"), 0, 1, "ToggleShadow");
		Bind(input.FindKey("T"), 0, 1, "ToggleTransparency");
		Bind(input.FindKey("W"), 0, 1, "ToggleWireframe");
		Bind(input.FindKey("Home"), 0, 0, "CameraReset");
		Bind(input.FindKey("Num 2"), 0, 0, "CameraScrollDown_On");
		Bind(input.FindKey("Num 2"), 1, 0, "CameraScrollDown_Off");
		Bind(input.FindKey("Num 4"), 0, 0, "CameraScrollLeft_On");
		Bind(input.FindKey("Num 4"), 1, 0, "CameraScrollLeft_Off");
		Bind(input.FindKey("Num 6"), 0, 0, "CameraScrollRight_On");
		Bind(input.FindKey("Num 6"), 1, 0, "CameraScrollRight_Off");
		Bind(input.FindKey("Num 8"), 0, 0, "CameraScrollUp_On");
		Bind(input.FindKey("Num 8"), 1, 0, "CameraScrollUp_Off");
		Bind(input.FindKey("Page Up"), 0, 0, "CameraZoomIn_On");
		Bind(input.FindKey("Page Up"), 1, 0, "CameraZoomIn_Off");
		Bind(input.FindKey("Page Down"), 0, 0, "CameraZoomOut_On");
		Bind(input.FindKey("Page Down"), 1, 0, "CameraZoomOut_Off");
		Bind(input.FindKey("Up"), 0, 0, "CameraTop");
		Bind(input.FindKey("Down"), 0, 0, "CameraDown");
		Bind(input.FindKey("Left"), 0, 0, "CameraAzimuthInc");
		Bind(input.FindKey("Right"), 0, 0, "CameraAzimuthDec");
		Bind(input.FindKey("X"), 0, 0, "CameraMouseRotate_On");
		Bind(input.FindKey("X"), 1, 0, "CameraMouseRotate_Off");
	}

	void GameHotKeys::Initialize() //469090.
	{
		LoadGameDefaults(); //468AE0; existing48 genuine registrations.
		JE::ClientHotKeys::Initialize(); //5998C0, actual fixed56AF30 object.
	}
}
