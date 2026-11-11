#include "game/HotKeysOptionsInterface.hpp"
#include "gui/Label.hpp"
#include "gui/Picture.hpp"

namespace F3
{
	HotKeyWindowFormat::HotKeyWindowFormat() //445C10.
	{
		//All three strings are live before the original literal assignments.
		skin = "DefaultInterface.ini";
		font = "arial";
		texture = "";
		fontSize = 10;
		fontFlags = 0;
		textColor = 0xFF624E14;
		x = 32;
		y = 60;
		width = 140;
		height = 24;
		frameVisible = 0;
		alignment = 0;
		verticalAlignment = 2;
	}

	HotKeyWindowFormat::~HotKeyWindowFormat() //445690.
	{
	}

	HotKeyRow::HotKeyRow() //445B50.
	{
		//Both strings are constructed before original pointer publication/assignment.
		keyLabel = NULL;
		descriptionLabel = NULL;
		cell = NULL;
		editing = 0;
		display = "";
		command = "";
		scan = -1;
		modifiers = 0;
	}

	HotKeyRow::~HotKeyRow() //445640.
	{
		//No deletes here: cell/description are background children, keyLabel
		//is a cell child. Preserve the actual GUI graph's ownership policy.
	}

	HotKeysOptionsInterface::HotKeysOptionsInterface(const std::string& name)
		: JE::EngineInterface(name), GameInterface(name) //4468C0.
	{
		//All three actual formatting-record constructors precede resource load.
		LoadResource("Options_HotKeys.INT");
		background = static_cast< ::GUI::Picture*>(FindWindow("picBackground"));
		rows.clear();
		::GUI::Label* label = static_cast< ::GUI::Label*>(FindWindow("lblHotKeyDesc"));
		if (label)
		{
			//Original446B45..446C54 /446ED3..446FFA reads all widget values
			//before publishing the captured record, then hides the template.
			LONG x, y, width, height, fontSize;
			DWORD fontFlags;
			std::string skin, font;
			label->GetAbsolutePosition(x, y);
			label->GetSize(width, height);
			label->GetSkin(skin);
			label->GetFont(font, &fontFlags, &fontSize, NULL);
			DWORD textColor = label->GetTextColor(); //Actual getter+16C.
			BYTE frameVisible = label->GetFrameVisible();
			DWORD alignment = label->GetAlignment(); //Actual getter+18C.
			DWORD verticalAlignment = label->GetVerticalAlignment(); //Getter+194.
			descriptionFormat.skin = skin;
			descriptionFormat.font = font;
			descriptionFormat.fontSize = fontSize;
			descriptionFormat.fontFlags = fontFlags;
			descriptionFormat.textColor = textColor;
			descriptionFormat.x = x;
			descriptionFormat.y = y;
			descriptionFormat.width = width;
			descriptionFormat.height = height;
			descriptionFormat.frameVisible = frameVisible;
			descriptionFormat.alignment = alignment;
			descriptionFormat.verticalAlignment = verticalAlignment;
			label->SetHidden(1);
		}
		::GUI::Picture* picture = static_cast< ::GUI::Picture*>(FindWindow("picHotKeyCell"));
		if (picture)
		{
			LONG x, y, width, height;
			std::string skin, texture;
			picture->GetAbsolutePosition(x, y);
			picture->GetSize(width, height);
			picture->GetSkin(skin);
			picture->GetTextureName(texture);
			BYTE frameVisible = picture->GetFrameVisible();
			cellFormat.skin = skin;
			cellFormat.texture = texture;
			cellFormat.x = x;
			cellFormat.y = y;
			cellFormat.width = width;
			cellFormat.height = height;
			cellFormat.frameVisible = frameVisible;
			picture->SetHidden(1);
		}
		label = static_cast< ::GUI::Label*>(FindWindow("lblHotKey"));
		if (label)
		{
			//Original446B45..446C54 /446ED3..446FFA reads all widget values
			//before publishing the captured record, then hides the template.
			LONG x, y, width, height, fontSize;
			DWORD fontFlags;
			std::string skin, font;
			label->GetAbsolutePosition(x, y);
			label->GetSize(width, height);
			label->GetSkin(skin);
			label->GetFont(font, &fontFlags, &fontSize, NULL);
			DWORD textColor = label->GetTextColor(); //Actual getter+16C.
			BYTE frameVisible = label->GetFrameVisible();
			DWORD alignment = label->GetAlignment(); //Actual getter+18C.
			DWORD verticalAlignment = label->GetVerticalAlignment(); //Getter+194.
			keyLabelFormat.skin = skin;
			keyLabelFormat.font = font;
			keyLabelFormat.fontSize = fontSize;
			keyLabelFormat.fontFlags = fontFlags;
			keyLabelFormat.textColor = textColor;
			keyLabelFormat.x = x;
			keyLabelFormat.y = y;
			keyLabelFormat.width = width;
			keyLabelFormat.height = height;
			keyLabelFormat.frameVisible = frameVisible;
			keyLabelFormat.alignment = alignment;
			keyLabelFormat.verticalAlignment = verticalAlignment;
			label->SetHidden(1);
		}

		//Exact authored order/names; native447DD0 uses CameraZoomIn_On.
		AddRow("AttackModeCycle", "Attack Mode Cycle");
		AddRow("AttackModeSelect", "Attack Mode Select");
		AddRow("CharacterRecordWindow", "Character Record");
		AddRow("FeedbackWindow", "Feedback/Map Toggle");
		AddRow("GameOptions", "Game Options");
		AddRow("HotKeyOptions", "Hotkey Options");
		AddRow("InventoryWindow", "Inventory");
		AddRow("LoadGameOptions", "Load/Save Menu");
		AddRow("MultiplayerOptions", "Multiplayer Options");
		AddRow("PipBoy", "Pip Boy");
		AddRow("SneakToggle", "Sneak Toggle");
		AddRow("UseSkill", "Use Skill");
		AddRow("VideoOptions", "Video Options");
		AddRow("WeaponLinkToggle", "Weapon Link Toggle");
		AddRow("WeaponToggle", "Weapon Toggle");
		AddRow("CameraMouseRotate_On", "Free Camera");
		AddRow("CameraReset", "Home Camera");
		AddRow("CameraDown", "Lower Camera");
		AddRow("CameraScrollDown_On", "Pan Down");
		AddRow("CameraScrollLeft_On", "Pan Left");
		AddRow("CameraScrollRight_On", "Pan Right");
		AddRow("CameraScrollUp_On", "Pan Up");
		AddRow("CameraTop", "Raise Camera");
		AddRow("CameraAzimuthInc", "Rotate Left");
		AddRow("CameraAzimuthDec", "Rotate Right");
		AddRow("CameraZoomIn_On", "Zoom In");
		AddRow("CameraZoomOut_On", "Zoom Out");
		AddRow("ActionbarWindow", "Actionbar (dbg)");
		AddRow("ConsoleWindow", "Console Win (dbg)");
		AddRow("ToggleBilinearFiltering", "Bilinear Filt. (dbg)");
		AddRow("ToggleFullscreen", "Fullscreen (dbg)");
		AddRow("ToggleStatistics", "Statistics (dbg)");
		AddRow("Kill", "Kill (dbg)");
		AddRow("ToggleLighting", "Toggle Lighting (dbg)");
		AddRow("RequestNewGame", "New Game (dbg)");
		AddRow("ToggleOctreeDrawing", "Toggle Octree (dbg)");
		AddRow("MemoryPoolTest", "Memory Test (dbg)");
		AddRow("ToggleShadow", "Toggle Shadow (dbg)");
		AddRow("ToggleTransparency", "Transparency (dbg)");
		AddRow("ToggleWireframe", "Wireframe (dbg)");
		//4461D0 is a separate opening hook; constructor does not reload keys.
	}

	HotKeysOptionsInterface::~HotKeysOptionsInterface() //445740.
	{
		for (std::list<HotKeyRow*>::iterator it = rows.begin(); it != rows.end(); ++it)
			if (*it)
				delete *it;
		rows.clear();
		//Styles then GameInterface/GUI::Interface perform normal destruction.
	}

	void HotKeysOptionsInterface::AddRow(const std::string& command,
		const std::string& description) //4461E0; physical RET8.
	{
		LONG count = static_cast<LONG>(rows.size());
		if (count >= 45)
			return;
		LONG columnOffset = 276 * (count / 15);
		LONG row = count % 15;
		LONG rowOffset = 27 * row + 19 * (row / 3);
		HotKeyRow* binding = new HotKeyRow;
		::GUI::Label* label = new ::GUI::Label(3);
		binding->descriptionLabel = label;
		background->AddChild(label);
		label->ApplySkin(descriptionFormat.skin);
		label->SetFont(descriptionFormat.font, descriptionFormat.fontFlags, descriptionFormat.fontSize, 1);
		label->SetTextColor(descriptionFormat.textColor);
		label->SetAlignment(descriptionFormat.alignment);
		label->SetVerticalAlignment(descriptionFormat.verticalAlignment);
		label->SetText(description);
		label->SetFrameVisible(descriptionFormat.frameVisible);
		label->Resize(descriptionFormat.width, descriptionFormat.height);
		label->SetAbsolutePosition(descriptionFormat.x + columnOffset, descriptionFormat.y + rowOffset);

		::GUI::Picture* cell = new ::GUI::Picture(2);
		binding->cell = cell;
		background->AddChild(cell);
		cell->ApplySkin(cellFormat.skin);
		cell->SetFrameVisible(cellFormat.frameVisible);
		cell->LoadTexture(cellFormat.texture, 0, 0, 0, -1, -1, 1);
		cell->Resize(cellFormat.width, cellFormat.height);
		cell->SetAbsolutePosition(cellFormat.x + columnOffset, cellFormat.y + rowOffset);

		label = new ::GUI::Label(3);
		binding->keyLabel = label;
		cell->AddChild(label);
		//Original key Label is positioned absolutely despite its cell parent.
		label->ApplySkin(keyLabelFormat.skin);
		label->SetFont(keyLabelFormat.font, keyLabelFormat.fontFlags, keyLabelFormat.fontSize, 1);
		label->SetTextColor(keyLabelFormat.textColor);
		label->SetAlignment(keyLabelFormat.alignment);
		label->SetVerticalAlignment(keyLabelFormat.verticalAlignment);
		label->SetText("");
		label->SetFrameVisible(keyLabelFormat.frameVisible);
		label->Resize(keyLabelFormat.width, keyLabelFormat.height);
		label->SetAbsolutePosition(keyLabelFormat.x + columnOffset, keyLabelFormat.y + rowOffset);
		binding->command = command;
		rows.push_back(binding);
	}
}
