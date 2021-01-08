//
// SETTINGS.CPP: Virtual Jaguar configuration loading/saving support
//
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JLH  01/16/2010  Created this log
// JLH  02/23/2013  Finally removed commented out stuff :-P
// JPM  09/08/2017  Added erase settings functions
//

#include "settings.h"
#include <QtCore/QSettings>

// Global variables

VJSettings vjs;


const char *ES[] = {	"", "all", "ui", "alpine", "debugger"	};


// Erase the settings by name
bool EraseSettings(char *Setting)
{
	size_t i;

	// Point on the emulator settings
	QSettings settings("Underground Software", "Virtual Jaguar");

	// Settings detection
	if (strcmp(Setting, "all") == 0)
	{
		settings.remove("");
		return true;
	}
	else
	{
		for (i = 2; i < SETTINGS_END; i++)
		{
			if (strcmp(ES[i], Setting) == 0)
			{
				settings.beginGroup(Setting);
				settings.remove("");
				settings.endGroup();
				return true;
			}
		}
	}

	return false;
}

