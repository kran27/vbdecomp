#pragma once

#include <windows.h>
#include <string>

namespace JE
{
	//Original304-byte profile at DialogueInterface+8/+312 under real VC7 strings.
	//These normal host members are not an old-layout overlay or ABI claim.
	struct DialogueAppearance
	{
		DialogueAppearance(); //5931B0, actual engine defaults before game template extraction.
		std::string windowSkin;
		LONG windowWidth, windowHeight;
		DWORD windowAlpha;
		std::string portraitSkin;
		LONG portraitX, portraitY, portraitWidth, portraitHeight;
		BYTE portraitFrameVisible;
		std::string nameLabelSkin, nameLabelFontName;
		LONG nameLabelFontSize;
		DWORD nameLabelFontFlags;
		LONG nameLabelX, nameLabelY, nameLabelWidth, nameLabelHeight;
		std::string textLabelSkin, textLabelFontName;
		LONG textLabelFontSize;
		DWORD textLabelFontFlags;
		LONG textLabelX, textLabelY, textLabelWidth, textLabelHeight;
		DWORD textValue100;
		LONG textBottomMargin;
		std::string modelSkin;
		LONG modelX, modelY, modelWidth, modelHeight;
		BYTE modelFrameVisible;
	};
}
