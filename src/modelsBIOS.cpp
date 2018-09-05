//
// modelsBIOS.cpp - Models and BIOS handler
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  09/04/2018  Created this file
//


#include "string.h"
#include "settings.h"
#include "jagbios.h"
#include "jagbios2.h"
#include "jagcdbios.h"
#include "jagstub1bios.h"
#include "jagstub2bios.h"
#include "memory.h"


typedef struct InfosBIOS
{
	void *ptrBIOS;
	size_t sizeBIOS;
	size_t intBIOS;
}
S_InfosBIOS;


S_InfosBIOS TabInfosBIOS[]	=
{
	{ NULL, 0, BT_NULL },
	{ jaguarBootROM, 0x20000, BT_K_SERIES },
	{ jaguarBootROM2, 0x20000, BT_M_SERIES },
	{ jaguarDevBootROM1, 0x20000, BT_STUBULATOR_1 },
	{ jaguarDevBootROM2, 0x20000, BT_STUBULATOR_2 }
};


// Select a BIOS
// A valid default BIOS will be selected in case of no valid BIOS has been requested
bool SelectBIOS(int indexbios)
{
	int IndexBIOS;

#if 1
	// Get the BIOS selected in the configuration tab
	if (!indexbios)
	{
		indexbios = vjs.biosType;
	}
#else
	indexbios = 10;
#endif

	// Check if a default BIOS is required
	if (!indexbios)
	{
		// Get default BIOS based on the Jaguar model
		switch (vjs.jaguarModel)
		{
		case JAG_K_SERIES:
			indexbios = BT_K_SERIES;
			break;

		case JAG_M_SERIES:
			indexbios = BT_M_SERIES;
			break;

		default:
			break;
		}
	}

	// look for the requested BIOS
	IndexBIOS = (sizeof(TabInfosBIOS) / sizeof(S_InfosBIOS));
	while (TabInfosBIOS[--IndexBIOS].intBIOS && (TabInfosBIOS[IndexBIOS].intBIOS != indexbios));

	// Put BIOS in memory or return if no BIOS exist (but it should never happen)
	if (IndexBIOS)
	{
		memcpy(jagMemSpace + 0xE00000, TabInfosBIOS[IndexBIOS].ptrBIOS, TabInfosBIOS[IndexBIOS].sizeBIOS);
		return true;
	}
	else
	{
		return false;
	}
}


// 
bool SetBIOS(void)
{
	memcpy(jaguarMainRAM, jagMemSpace + 0xE00000, 8);
	return true;
}

