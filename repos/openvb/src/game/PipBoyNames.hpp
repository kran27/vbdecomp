#ifndef F3_PIPBOY_NAMES_HPP
#define F3_PIPBOY_NAMES_HPP

#include <windows.h>

// Private PipBoy table owners; included once by PipBoyCompilation.cpp.
// Original writable .data: 6F3318/6F3388/6F33F8/6F3468.
namespace
{
	struct AlarmEntry
	{
		const char* name;
		LONG index;
	};
	AlarmEntry AlarmNames[14] = //6F3318, actual name/index pairs.
	{
		{"lbl10min", 0}, {"lbl30min", 1}, {"lbl1hr", 2}, {"lbl2hrs", 3},
		{"lbl3hrs", 4}, {"lbl4hrs", 5}, {"lbl5hrs", 6}, {"lbl6hrs", 7},
		{"lblMorning", 8}, {"lblNoon", 9}, {"lblEvening", 10}, {"lblMidnight", 11},
		{"lblHealed", 12}, {"lblPartyHealed", 13}
	};
	const char* TaskLabelNames[14][2] =
	{
		{"lblTask11", "lblTask21"}, {"lblTask12", "lblTask22"},
		{"lblTask13", "lblTask23"}, {"lblTask14", "lblTask24"},
		{"lblTask15", "lblTask25"}, {"lblTask16", "lblTask26"},
		{"lblTask17", "lblTask27"}, {"lblTask18", "lblTask28"},
		{"lblTask19", "lblTask29"}, {"lblTask1a", "lblTask2a"},
		{"lblTask1b", "lblTask2b"}, {"lblTask1c", "lblTask2c"},
		{"lblTask1d", "lblTask2d"}, {"lblTask1e", "lblTask2e"}
	};
	const char* MapLabelNames[14][2] = { //6F33F8, actual shared map labels.
		{"lblMaps11","lblMaps21"},{"lblMaps12","lblMaps22"},
		{"lblMaps13","lblMaps23"},{"lblMaps14","lblMaps24"},
		{"lblMaps15","lblMaps25"},{"lblMaps16","lblMaps26"},
		{"lblMaps17","lblMaps27"},{"lblMaps18","lblMaps28"},
		{"lblMaps19","lblMaps29"},{"lblMaps1a","lblMaps2a"},
		{"lblMaps1b","lblMaps2b"},{"lblMaps1c","lblMaps2c"},
		{"lblMaps1d","lblMaps2d"},{"lblMaps1e","lblMaps2e"}
	};
	const char* DownloadNames[14][2] = //6F3468, borrowed name pointers.
	{
		{"lblDownloads11", "lblDownloads21"}, {"lblDownloads12", "lblDownloads22"},
		{"lblDownloads13", "lblDownloads23"}, {"lblDownloads14", "lblDownloads24"},
		{"lblDownloads15", "lblDownloads25"}, {"lblDownloads16", "lblDownloads26"},
		{"lblDownloads17", "lblDownloads27"}, {"lblDownloads18", "lblDownloads28"},
		{"lblDownloads19", "lblDownloads29"}, {"lblDownloads1a", "lblDownloads2a"},
		{"lblDownloads1b", "lblDownloads2b"}, {"lblDownloads1c", "lblDownloads2c"},
		{"lblDownloads1d", "lblDownloads2d"}, {"lblDownloads1e", "lblDownloads2e"}
	};
}

#endif
