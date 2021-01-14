//
// ELFManager.cpp: HW Label manager
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM  02/08/2017  Created this file
// JPM  02/08/2017  HW Label support
//

#include <stdlib.h>
#include <string.h>
#include "libelf.h"
#include "gelf.h"
#include "log.h"
#include "ELFManager.h"


typedef enum {
	HWLABEL_NO_SIZE = 0,
	HWLABEL_8BITS = 1,
	HWLABEL_16BITS = 2,
	HWLABEL_32BITS = 4,
	HWLABEL_64BITS = 8
}HWLABELSIZE;

typedef enum {
	HWLABEL_NO_ACCESS = 0,
	HWLABEL_R = 0x1,
	HWLABEL_W = 0x2,
	HWLABEL_O = 0x4
}HWLABELACCESS;

typedef struct {
	size_t HWLABELAdr;
	const char *HWLABELSymbolName;
	const char *HWLABELFulllName;
	size_t HWLABELSize;
	size_t HWLABELAccess;
}HWLABELTab;

#define NBHWLABELS (sizeof(HWLABELTabSectionType) / sizeof(HWLABELTab))


// Memory map list based on the scans from the Version 2.4 - June 7, 1995
HWLABELTab	HWLABELTabSectionType[] =	{
// Internal Memory Map
	{	0xF00000, "MEMCON1", "Memory Configuration Register One", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF00002, "MEMCON2", "Memory Configuration Register Two", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF00004, "HC", "Horizontal Count", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF00006, "VC", "Vertical Count", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF00008, "LPH", "Horizontal Light-pen", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_O)	},
	{	0xF0000A, "LPV", "Vertical Light-pen", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_O)	},
	{	0xF00010, "OB[0]", "Object Code", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_O)	},
	{	0xF00012, "OB[1]", "Object Code", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_O)	},
	{	0xF00014, "OB[2]", "Object Code", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_O)	},
	{	0xF00016, "OB[3]", "Object Code", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_O)	},
	{	0xF00020, "OLP", "Object List Pointer", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00026, "OBF", "Object Processor flag", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00028, "VMODE", "Video Mode", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0002A, "BORD1", "Border Colour (Red & Green)", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0002C, "BORD2", "Border Colour (Blue)", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0002E, "HP", "Horizontal Period", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00030, "HBB", "Horizontal Blanking Begin", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00032, "HBE", "Horizontal Blanking End", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00034, "HS", "Horizontal Sync", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00036, "HVS", "Horizontal Vertical Sync", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00038, "HDB1", "Horizontal Display Begin 1", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0003A, "HDB2", "Horizontal Display Begin 2", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0003C, "HDE", "Horizontal Display End", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0003E, "VP", "Vertical Period", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00040, "VBB", "Vertical Blanking Begin", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00042, "VBE", "Vertical Blanking End", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00044, "VS", "Vertical Sync", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00046, "VDB", "Vertical Display Begin", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00048, "VDE", "Vertical Display End", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0004A, "VEB", "Vertical Equalization Begin", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0004C, "VEE", "Vertical Equalization End", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0004E, "VI", "Vertical Interrupt", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00050, "PIT[0]", "Programmable Interrupt Timer", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00052, "PIT[1]", "Programmable Interrupt Timer", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00054, "HEQ", "Horizontal equalization end", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00058, "BG", "Background Colour", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF000E0, "INT1", "CPU Interrupt Control Register", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF000E2, "INT2", "CPU Interrupt resume register", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF00400, "CLUT", "Colour Look-Up Table", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF00800, "LBUF", "Line Buffer A", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF01000, "LBUF", "Line Buffer B", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF01800, "LBUF", "Line Buffer selected for writing", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W) },
// Internal registers of the Graphics processor
	{	0xF02100, "G_FLAGS", "GPU Flags Register", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF02104, "G_MTXC", "Matrix Control Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02108, "G_MTXA", "Matrix Address Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0210C, "G_END", "Data Organisation Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02110, "G_PC", "GPU Program Counter", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF02114, "G_CTRL", "GPU Control/Status Register", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W)	},
	{	0xF02118, "G_HIDATA", "High Data Register", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W)	},
	{	0xF0211C, "G_REMAIN", "Divide unit remainder", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_O) },
	{	0xF0211C, "G_DIVCTRL", "Divide unit Control", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
// List of all the externally accessible locations within the Blitter.
	{	0xF02200, "A1_BASE", "A1 Base Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02204, "A1_FLAGS", "A1 Flags Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02208, "A1_CLIP", "A1 Clipping Size", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0220C, "A1_PIXEL", "A1 Pixel Pointer", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W)	},
	{	0xF02210, "A1_STEP", "A1 Step Value", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02214, "A1_FSTEP", "A1 Step Fraction Value", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02218, "A1_FPIXEL", "A1 Pixel Pointer Fraction", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF0221C, "A1_INC", "A1 Increment", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02220, "A1_FINC", "A1 Increment Fraction", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02224, "A2_BASE", "A2 Base Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02228, "A2_FLAGS", "A2 Flags Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0222C, "A2_MASK", "A2 Window Mask", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02230, "A2_PIXEL", "A2 Pixel Pointer", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF02234, "A2_STEP", "A2 Step Value", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02238, "B_CMD", "Command Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02238, "B_CMD", "Status Register", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_O) },
	{	0xF0223C, "B_COUNT", "Counters Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02240, "B_SRCD", "Source Data Register", HWLABEL_64BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02248, "B_DSTD", "Destination Data Register", HWLABEL_64BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02250, "B_DSTZ", "Destination Z Register", HWLABEL_64BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02258, "B_SRCZ1", "Source Z Register 1", HWLABEL_64BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02260, "B_SRCZ2", "Source Z Register 2", HWLABEL_64BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02268, "B_PATD", "Pattern Data Register", HWLABEL_64BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02270, "B_IINC", "Intensity Increment", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02274, "B_ZINC", "Z Increment", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02278, "B_STOP", "Collision control", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0227C, "B_I3", "Intensity 3", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02280, "B_I2", "Intensity 2", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02284, "B_I1", "Intensity 1", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02288, "B_I0", "Intensity 0", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF0228C, "B_Z3", "Z 3", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02290, "B_Z2", "Z 2", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02294, "B_Z1", "Z 1", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF02298, "B_Z0", "Z 0", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
// GPU Ram base
	//{	0xF03000, "GPU_RAMBASE", "Local RAM base", HWLABEL_8BITS, (HWLABEL_R | HWLABEL_W) },
// Frequency dividers
	{	0xF10010, "CLK1", "Processor clock divider", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF10012, "CLK2", "Video clock divider", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF10014, "CLK3", "Chroma clock divider", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
// Programmable Timers
	{	0xF10000, "JPIT1", "Timer 1 Pre-scaler", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF10004, "JPIT3", "Timer 2 Pre-scaler", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF10002, "JPIT2", "Timer 1 Divider", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF10006, "JPIT4", "Timer 2 Divider", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	//{	0xF10036, "JPIT1", "Timer 1 Pre-scaler", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_O) },
	//{	0xF10038, "JPIT2", "Timer 1 Divider", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_O) },
	//{	0xF1003A, "JPIT3", "Timer 2 Pre-scaler", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_O) },
	//{	0xF1003C, "JPIT4", "Timer 2 Divider", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_O) },
// Interrupts
	{	0xF10020, "JINTCTRL", "Interrupt Control Register", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_W) },
// JERRY Pulse Width Modulation DACs
	//{	0xF1A140, "DAC1", "Left DAC", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	//{	0xF1A144, "DAC2", "Right DAC", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
// Synchronous Serial Interface
	{	0xF1A150, "SCLK", "Serial Clock Frequency", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF1A154, "SMODE", "Serial Mode", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF1A148, "R_DAC", "Right transmit data (to DACs)", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF1A14C, "L_DAC", "Left transmit data (to DACs)", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF1A148, "LTXD", "Left transmit data (to I2S)", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF1A14C, "RTXD", "Right transmit data (to I2S)", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF1A148, "LRXD", "Left receive data (to I2S)", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_O) },
	{	0xF1A14C, "RRXD", "Right receive data (to I2S)", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_O) },
	{	0xF1A150, "SSTAT", "Serial Status", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_O) },
// Asynchronous Serial Interface (ComLynx and Midi)
	{	0xF10034, "ASICLK", "Asynchronous Serial Interface Clock", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF10032, "ASICTRL", "Asynchronous Serial Control", HWLABEL_16BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF10032, "ASISTAT", "Asynchronous Serial Status", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_O) },
	{	0xF10030, "ASIDATA", "Asynchronous Serial Data", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_W) },
// Joystick Interface
	{	0xF14000, "JOYSTICK", "Joystick register", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF14002, "JOYBUTS", "Button register", HWLABEL_16BITS, (HWLABEL_R | HWLABEL_W) },
// Internal Registers
	{	0xF1A100, "D_FLAGS", "DSP Flags Register", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF1A104, "D_MTXC", "DSP Matrix Control Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF1A108, "D_MTXA", "DSP Matrix Address Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF1A10C, "D_END", "DSP Data Organisation Register", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF1A110, "D_PC", "DSP Program Counter", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF1A114, "D_CTRL", "DSP Control/Status Register", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF1A118, "D_MOD", "Modulo instruction mask", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF1A11C, "D_REMAIN", "Divide unit remainder", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_W) },
	{	0xF1A11C, "D_DIVCTRL", "Divide unit Control", HWLABEL_32BITS, (HWLABEL_W | HWLABEL_O) },
	{	0xF1A120, "D_MACHI", "Multiply & Acccumulate High Bits", HWLABEL_32BITS, (HWLABEL_R | HWLABEL_O) },
// End of the Memory map list
//	{	(size_t)-1, NULL, NULL, HWLABEL_NO_SIZE, HWLABEL_NO_ACCESS }
};


// Get Symbol name from his address
char *HWLABELManager_GetSymbolnameFromAdr(size_t Adr)
{
	size_t i;

	if ((Adr >= 0xF00000) && (Adr < 0xF1A124))
	{
		for (i = 0; i < NBHWLABELS; i++)
		{
			if ((HWLABELTabSectionType[i].HWLABELAdr == Adr))
			{
				return (char *)HWLABELTabSectionType[i].HWLABELSymbolName;
			}
		}
	}

	return NULL;

	//while ((HWLABELTabSectionType[i].HWLABELAdr != Adr) && (HWLABELTabSectionType[i++].HWLABELAdr != (size_t)-1));
	//return (char *)HWLABELTabSectionType[i].HWLABELSymbolName;
}

