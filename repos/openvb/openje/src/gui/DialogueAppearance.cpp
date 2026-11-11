#include "gui/DialogueAppearance.hpp"

namespace JE
{
	DialogueAppearance::DialogueAppearance() //5931B0.
	{
		windowSkin = "DefaultInterface.ini";
		windowWidth = 320; windowHeight = 100; windowAlpha = 255;
		portraitSkin = "DefaultInterface.ini";
		portraitX = 4; portraitY = 4;
		portraitWidth = 32; portraitHeight = 32;
		portraitFrameVisible = 1;
		nameLabelSkin = "DefaultInterface.ini"; nameLabelFontName = "Arial";
		nameLabelFontSize = 12; nameLabelY = 12;
		nameLabelWidth = windowWidth - 52;
		nameLabelFontFlags = 1; nameLabelX = 44; nameLabelHeight = 16;
		textLabelSkin = "DefaultInterface.ini"; textLabelFontName = "Arial";
		textLabelFontSize = 8; textLabelX = 8; textLabelFontFlags = 0;
		textLabelY = 40; textLabelWidth = windowWidth - 16; textLabelHeight = 16;
		textValue100 = 0; textBottomMargin = 6;
		modelSkin = "DefaultInterface.ini";
		modelWidth = 96; modelHeight = 96;
		modelX = 280; modelY = 4; modelFrameVisible = 1;
	}
}
