//
// tracyprofiler.cpp - Profiler with Tracy
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM  10/29/2025       Created this file
// JPM   Oct./2025       Added pause feature, better control for the Tracy profiler feed and memory allocation tracking
//

//
//#define TracyFunction
//
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tracyprofiler.h"

//
#undef TracyLine
#undef TracyFunction
#undef TracyFile

// message colour codes
#define COLOUR_DEFAULT	0xFFFFFFFF
#define COLOUR_INFO		0xFFFFFF80
#define COLOUR_WARNING	0xFFFFA000
#define COLOUR_ERROR	0xFF8080FF
#define COLOUR_SUCCESS	0xFF00FF00
#define COLOUR_DEBUG	0xFF808080

constexpr double M68K_CLOCK_HZ = 13290000.0; // 13.29 MHz


//
TracyProfiler::TracyProfiler(void)
{
	tracyPaused = true;

	// name the application in Tracy, is displayed in the Trace information
	TracyCAppInfo("Virtual Jaguar Rx - Profiler", strlen("Virtual Jaguar Rx - Profiler"));
	//TracyCSetThreadName("M68K CPU");
	// displayed the message in the Tracy profiler messages list
	TracyCMessageC("=== Atari Jaguar: 68000  ===", 28, COLOUR_DEFAULT);
	//TracyCPlotConfig("68000 cycles", TracyPlotFormatNumber, true, true, 0xFFFFFF80);
}


// Pause or resume the Tracy profiler
void TracyProfiler::Pause(bool pause)
{
	tracyPaused = !pause;
}


// Add a malloc event for the Tracy memory allocation
void TracyProfiler::M68Kmalloc(TracyCZoneCtx* zoneCtx, unsigned int ptr, unsigned int size, int depth)
{
	if (!tracyPaused)
	{
		char buffer[128];
		if (ptr)
		{
			// check potential weid size
			if (size > 0x200000)
			{
				// display message for a weird size
				sprintf(buffer, "M68K malloc: ptr=0x%06x size=%u bytes (weird size, profiler may have an issue)", ptr, size);
				TracyCMessageC(buffer, strlen(buffer), COLOUR_DEBUG);
			}
			else
			{
				// display message for a non-null pointer
				sprintf(buffer, "M68K malloc: ptr=0x%06x size=%u bytes", ptr, size);
				TracyCMessageC(buffer, strlen(buffer), COLOUR_SUCCESS);
			}
			// add the pointer address in the memory allocation
			zoneCtx->active = (depth >= 0) ? TracyCAlloc((void*)ptr, size, 0, "68000 DRAM", (zoneCtx->id = depth)), true : false;
		}
		else
		{
			// display message for an unknown pointer
			sprintf(buffer, "M68K malloc: ptr=0x%06x size=%u bytes (memory allocation error)", ptr, size);
			TracyCMessageC(buffer, strlen(buffer), COLOUR_ERROR);
			zoneCtx->active = false;
		}
	}
}


// Add the free event for the Tracy memory allocation
void TracyProfiler::M68Kfree(TracyCZoneCtx* zoneCtx, unsigned int ptr)
{
	if (!tracyPaused)
	{
		char buffer[128];
		if (zoneCtx >= 0)
		{
			// display message for a null or an existing pointer
			sprintf(buffer, "M68K free: ptr=0x%06x", ptr);
			unsigned int colour = ptr ? COLOUR_SUCCESS : COLOUR_WARNING;
			TracyCMessageC(buffer, strlen(buffer), colour);
			// free the non-null pointer address in the memory allocation
			(ptr && zoneCtx && zoneCtx->active) ? TracyCFree((void*)ptr, 0, "68000 DRAM", zoneCtx->id), (zoneCtx->active = false) : false;
		}
		else
		{
			// display message for an unknown pointer
			sprintf(buffer, "M68K free: ptr=0x%06x (unknown)", ptr);
			TracyCMessageC(buffer, strlen(buffer), COLOUR_ERROR);
		}
	}
}


// Start a Tracy zone for the 68000 function
// The plot colors will depend on the plot's name (yellow)
void TracyProfiler::M68Kenter(TracyCZoneCtx* zoneCtx, char *functionName, char* filename, unsigned int startCycle)
{
	if (!tracyPaused)
	{
		// clear the file & line wording dedicated to the function name
		unsigned int TracyLine = 0;
		char* TracyFile = (char*)"";
		// colour for the function's name zone
		TracyCZoneC(ctx, 0xBEBE70, true);
		// display the function's name
		TracyCZoneName(ctx, functionName, strlen(functionName));
		//
		*zoneCtx = ctx;
		// start cycle plot
		TracyCPlot("M68K start cycle", (double)startCycle);
	}
	else
	{
		*zoneCtx = { 0 };
	}
}


// End the tracy zone for the 68000 function
// The plot colors will depend on the plot's name (yellow)
void TracyProfiler::M68Kleave(TracyCZoneCtx* pZone, unsigned int usedCycles)
{
	if (!tracyPaused || (pZone->id && pZone->active))
	{
		char text[64];

		// display the used cycles in the function zone text
		snprintf(text, sizeof(text), "%u cycles", usedCycles);
		TracyCZoneText(*pZone, text, strlen(text));

		TracyCPlot("M68K cycles per function", (double)usedCycles);
		TracyCPlot("M68K function time (\xC2\xB5s)", (usedCycles / M68K_CLOCK_HZ) * 1e6);
		TracyCPlot("M68K function time (ms)", (usedCycles / M68K_CLOCK_HZ) * 1e3);
		TracyCZoneEnd(*pZone);
	}
}


//
TracyProfiler::~TracyProfiler(void)
{

}
