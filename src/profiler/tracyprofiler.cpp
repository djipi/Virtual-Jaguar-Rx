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


constexpr double M68K_CLOCK_HZ = 13290000.0; // 13.29 MHz


//
const char* TracyProfilerNoFilename = "?";


//
TracyProfiler::TracyProfiler(void)
{
	TracyCAppInfo("Virtual Jaguar Rx - Profiler", strlen("Virtual Jaguar Rx - Profiler"));
	TracyCSetThreadName("68000");
}


// Start a tracy zone for the 68000 function
// The plot colors will depend on the plot's name (yellow)
void TracyProfiler::M68Kenter(TracyCZoneCtx* zoneCtx, char *functionName, char* filename, unsigned int startCycle)
{
	unsigned int TracyLine = 0;
	//char* TracyFunction = functionName;
	char* TracyFile = (char*)"";			// filename;
	TracyCZoneC(ctx, 0xFFFFFF80, true);
	if (functionName)
	{
		TracyCZoneName(ctx, functionName, strlen(functionName));
		//TracyCZoneText(ctx, functionName, strlen(functionName));
	}
	else
	{
		TracyCZoneName(ctx, TracyProfilerNoFilename, strlen(TracyProfilerNoFilename));
		//TracyCZoneText(ctx, TracyProfilerNoFilename, strlen(TracyProfilerNoFilename));
	}
	*zoneCtx = ctx;
	TracyCPlot("M68K start cycle", (double)startCycle);
}


// End the tracy zone for the 68000 function
// The plot colors will depend on the plot's name (yellow)
void TracyProfiler::M68Kleave(TracyCZoneCtx* pZone, unsigned int usedCycles)
{
	char text[64];

	// display the used cycles in the function zone text
	snprintf(text, sizeof(text), "%u cycles", usedCycles);
	TracyCZoneText(*pZone, text, strlen(text));

	TracyCPlot("M68K cycles per function", (double)usedCycles);
	TracyCPlot("M68K function time (µs)", (usedCycles / M68K_CLOCK_HZ) * 1e6);
	TracyCPlot("M68K function time (ms)", (usedCycles / M68K_CLOCK_HZ) * 1e3);
	TracyCZoneEnd(*pZone);
}


//
TracyProfiler::~TracyProfiler(void)
{

}
