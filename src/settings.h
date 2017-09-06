//
// settings.h: Header file
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JPM  06/19/2016  Soft debugger support
//

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

// MAX_PATH isn't defined in stdlib.h on *nix, so we do it here...
#ifdef __GCCUNIX__
#include <limits.h>
#define MAX_PATH		_POSIX_PATH_MAX
#else
#include <stdlib.h>				// for MAX_PATH on MinGW/Darwin
// Kludge for Win64
#ifndef MAX_PATH
#define MAX_PATH _MAX_PATH		// Urgh.
#endif
#endif
#include <stdint.h>

#define MaxMemory1BrowserWindow		4

// Settings struct
struct VJSettings
{
	bool useJoystick;
	int32_t joyport;											// Joystick port
	bool hardwareTypeNTSC;										// Set to false for PAL
	bool useJaguarBIOS;
	bool GPUEnabled;
	bool DSPEnabled;
	bool usePipelinedDSP;
	bool fullscreen;
	bool useOpenGL;
	uint32_t glFilter;
	bool hardwareTypeAlpine;
	bool softTypeDebugger;										// Soft type debugger
	bool audioEnabled;
	uint32_t frameSkip;
	uint32_t renderType;
	uint32_t refresh;
	bool allowWritesToROM;
	uint32_t biosType;
	uint32_t nbrdisasmlines;
	bool disasmopcodes;
	bool displayHWlabels;
	bool useFastBlitter;
	bool displayFullSourceFilename;
	size_t nbrmemory1browserwindow;								// Number of memory browser windows
	size_t DRAM_size;											// DRAM size

	// Keybindings in order of U, D, L, R, C, B, A, Op, Pa, 0-9, #, *

	uint32_t p1KeyBindings[21];
	uint32_t p2KeyBindings[21];

	// Paths

	char ROMPath[MAX_PATH];
	char jagBootPath[MAX_PATH];
	char CDBootPath[MAX_PATH];
	char EEPROMPath[MAX_PATH];
	char alpineROMPath[MAX_PATH];
	char debuggerROMPath[MAX_PATH];
	char absROMPath[MAX_PATH];
};

// Render types

enum { RT_NORMAL = 0, RT_TV = 1 };

// BIOS types

enum { BT_K_SERIES, BT_M_SERIES, BT_STUBULATOR_1, BT_STUBULATOR_2 };

// Exported variables

extern VJSettings vjs;

#endif	// __SETTINGS_H__
