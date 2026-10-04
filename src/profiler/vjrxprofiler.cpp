//
// vjrxprofiler.cpp - Profiler with VJRx
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  mm/dd/yyyy  What
// ---  ----------  -----------------------------------------------------------
// JPM   Dec./2025  Created this file

#include "profiler/vjrxprofiler.h"

// M68K variables for the VJRx profiler
static constexpr double M68K_CLOCK_HZ = 13290000.0;					// 13.29 MHz
static double NANOS_PER_CYCLE = 1000000000.0 / M68K_CLOCK_HZ;		// M68K @ 13.29 MHz: 1 cycle is more or less 75.244 nanoseconds


//
VJRxProfiler::VJRxProfiler(void) :
NbM68KRecord(0),
Paused(true)
{
	memset(M68KRecord, 0, sizeof(M68KRecord));
}


// Pause, or resume, the VJRx profiler
// Arguments:
// true: pause, false: resume
void VJRxProfiler::Pause(bool pause)
{
	Paused = !pause;
}


// Reset the index
void VJRxProfiler::RAZIndex(void* index)
{
	if (((VJRxZoneCtx*)index)->active)
	{
		switch (((VJRxZoneCtx*)index)->type)
		{
		case VJRxProfilerType_t::Record:
			// refresh the profiler window
			VJRxProfilerWin->RefreshContents(NbM68KRecord--, (VJRxProfilerWindow::VJRxProfilerData*)&M68KRecord[0]);
			// clear the record
			M68KRecord[(((VJRxZoneCtx*)index)->id)] = { 0 };
			break;

		default:
			break;
		}
		// mark as inactive
		((VJRxZoneCtx*)index)->active = false;
	}
	// reset id and type
	((VJRxZoneCtx*)index)->id = 0;
	((VJRxZoneCtx*)index)->type = VJRxProfilerType_t::Null;
}


//
bool VJRxProfiler::Start(void)
{
	VJRxProfilerWin = new VJRxProfilerWindow();
	VJRxProfilerWin->show();
	return false;
}


// Record a function profiling data
void VJRxProfiler::M68Krecord(void* ZoneCtx, char* functionName, char* filename, size_t linenumber, size_t callcount, size_t minCycles, size_t maxCycles)
{
	if (!Paused)
	{
		// look for the id
		size_t NumID = 0;
		while ((NumID < NbM68KRecord) && M68KRecord[NumID].FunctionName && ((!strcmp(functionName, M68KRecord[NumID].FunctionName) && (strlen(functionName) == strlen(M68KRecord[NumID].FunctionName))) ? false : true) && ++NumID);
		//
		if (!(((VJRxZoneCtx*)ZoneCtx)->active))
		{
			(NumID == NbM68KRecord) ? NbM68KRecord++, true : false;
			// new record
			M68KRecord[NumID].id = NumID;
			M68KRecord[NumID].FunctionName = functionName;
			M68KRecord[NumID].SourceFileName = filename;
			M68KRecord[NumID].LineNumber = linenumber;
			// set the zone context
			((VJRxZoneCtx*)ZoneCtx)->id = NumID;
			((VJRxZoneCtx*)ZoneCtx)->active = true;
			((VJRxZoneCtx*)ZoneCtx)->type = VJRxProfilerType_t::Record;
		}
		// update the record
		M68KRecord[NumID].CallCount = callcount;
		M68KRecord[NumID].maxCycles = maxCycles;
		M68KRecord[NumID].minCycles = minCycles;
#if 0
		M68KRecord[NumID].avgms = ((minCycles + maxCycles) / 2) * NANOS_PER_CYCLE / 1000000.0;
		M68KRecord[NumID].maxms = maxCycles * NANOS_PER_CYCLE / 1000000.0;
#endif
		// update the profiler window
		VJRxProfilerWin->UpdateContent(NumID, (VJRxProfilerWindow::VJRxProfilerData*)&M68KRecord[NumID]);
	}
}


//
VJRxProfiler::~VJRxProfiler(void)
{
	delete VJRxProfilerWin;
	VJRxProfilerWin = nullptr;
}
