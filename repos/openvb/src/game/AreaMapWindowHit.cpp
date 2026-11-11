#include "game/AreaMapWindow.hpp"
#include "game/AreaMapIcon.hpp"
#include "gui/Manager.hpp"

void AreaMapIcon::SetFlags(DWORD value) //41D190.
{
	if (value!=flags) { flags=value; dirty=1; }
}
DWORD AreaMapIcon::GetFlags() const { return flags; } //41D1B0.

DWORD AreaMapIcon::GetIndex() const //41D1F0.
{
	return index;
}

AreaMapIcon* AreaMapWindow::HitNoteIcon(LONG x, LONG y, LONG* index) //41E820.
{
	std::list<AreaMapIcon*>::iterator i = noteIcons.begin();
	const std::list<AreaMapIcon*>::iterator end = noteIcons.end();
	for (; i != end; ++i)
	{
		AreaMapIcon* icon = *i;
		LONG left, top, width, height;
		icon->GetRect(left, top, width, height);
		if (x >= left && x < left + width && y >= top && y < top + height)
		{
			if (index) *index = LONG(icon->GetIndex());
			return icon;
		}
	}
	return NULL;
}

JE::AreaMapNote* AreaMapWindow::HitNote(LONG x, LONG y, LONG* index) //41E8D0.
{
	AreaMapIcon* icon = HitNoteIcon(x, y, index);
	return icon ? icon->GetNote() : NULL;
}

AreaMapIcon* AreaMapWindow::HitPartyIcon(LONG x, LONG y, DWORD* index) //41E900.
{
	if (!showParty) return NULL;
	std::vector<AreaMapIcon*>::iterator i = partyIcons.begin();
	const std::vector<AreaMapIcon*>::iterator end = partyIcons.end();
	for (; i != end; ++i)
	{
		AreaMapIcon* icon = *i;
		LONG left, top, width, height;
		icon->GetRect(left, top, width, height);
		if (x >= left && x < left + width && y >= top && y < top + height)
		{
			if (index) *index = icon->GetIndex();
			return icon;
		}
	}
	return NULL;
}

void AreaMapWindow::UpdatePartyHover(LONG x,LONG y) //41F080.
{
	if (showParty)
	{
		float sx,sy; ::GUI::g_Manager.GetScale(sx,sy);
		x=LONG(static_cast<__int64>(x*sx));
		y=LONG(static_cast<__int64>(y*sy));
		std::vector<AreaMapIcon*>::iterator i=partyIcons.begin();
		BYTE found=0; overPartyIcon=0;
		for (;i!=partyIcons.end();++i)
		{
			AreaMapIcon* icon=*i; LONG ix,iy,width,height;
			icon->GetRect(ix,iy,width,height);
			if (!found && partyHoverEnabled && x>=ix && x<ix+width && y>=iy && y<iy+height)
			{ icon->SetFlags(icon->GetFlags()|1); found=1; overPartyIcon=1; }
			else icon->SetFlags(icon->GetFlags()&~DWORD(1));
		}
	}
}

void AreaMapWindow::UpdateNoteHover(LONG x,LONG y) //41EF80.
{
	float sx,sy; ::GUI::g_Manager.GetScale(sx,sy);
	x=LONG(static_cast<__int64>(x*sx));
	y=LONG(static_cast<__int64>(y*sy));
	BYTE found=0;
	for (std::list<AreaMapIcon*>::iterator i=noteIcons.begin();i!=noteIcons.end();++i)
	{
		AreaMapIcon* icon=*i; LONG ix,iy,width,height;
		icon->GetRect(ix,iy,width,height);
		if (!found && noteHoverEnabled && !overPartyIcon && x>=ix && x<ix+width && y>=iy && y<iy+height)
		{ icon->SetFlags(icon->GetFlags()|1); found=1; }
		else icon->SetFlags(icon->GetFlags()&~DWORD(1));
	}
}
