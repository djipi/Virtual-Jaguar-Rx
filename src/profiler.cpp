//
// profiler.cpp - Profiling information
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM  10/29/2025       Created this file
// JPM   Oct./2025       Added pause feature and flush data to the Tracy profiler
//

#include <stdio.h>
#include "debugger/DBGManager.h"
#include "profiler/tracyprofiler.h"
//#include "memory.h"
//#include "profiler.h"

#define M68K_PROFILER_MAX_ENTRIES	100000


// Structure to hold record profiling information
struct ProfilerRecordEntry_s {
	unsigned int PCFuncAdr;
	char* functionName;
	char* sourcefilename;
	unsigned int callCount;
	unsigned int minCycles;
	unsigned int maxCycles;
};

// Structure to hold current profiling information
struct ProfilerCurrentEntry_s {
	int previousIndex;
	unsigned int PCFuncAdr;
	char* functionName;
	char* sourcefilename;
	unsigned int currentCycles;
	unsigned int startCycles;
	unsigned int endCycles;
	TracyCZoneCtx tracyCtx;
};


// Arrays to hold profiling entries
ProfilerCurrentEntry_s m68kProfilerTable[M68K_PROFILER_MAX_ENTRIES];
ProfilerRecordEntry_s m68kProfilerTableRecord[M68K_PROFILER_MAX_ENTRIES];
// Index and count
int m68kProfilerEntryIndex, m68kProfilerEntryCountRecord;
bool M68KProfilerTableOverflow;
//
unsigned int M68KProfilerCycles;
bool M68Kdeferentry;
TracyProfiler *m68kTracyProfiler;


//
void ProfilerClear(void);


// Profiler initialization
void ProfilerInit(void)
{
	m68kTracyProfiler = new(TracyProfiler);
	ProfilerClear();
}


// Pause or resume the profiler
void ProfilerPause(bool pause)
{
	m68kTracyProfiler->Pause(pause);
}


// Profiler initialization
void ProfilerClear(void)
{
	// set variables
	m68kProfilerEntryIndex = -1;
	m68kProfilerEntryCountRecord = 0;
	M68Kdeferentry = true;
	M68KProfilerCycles = 0;
	M68KProfilerTableOverflow = false;

	// prepare the tables
	for (unsigned int i = 0; i < M68K_PROFILER_MAX_ENTRIES; i++)
	{
		m68kProfilerTable[i].PCFuncAdr = m68kProfilerTableRecord[i].PCFuncAdr = 0;
		m68kProfilerTable[i].functionName = m68kProfilerTable[i].sourcefilename = nullptr;
		m68kProfilerTable[i].previousIndex = -1;
		m68kProfilerTable[i].currentCycles = m68kProfilerTable[i].startCycles = m68kProfilerTable[i].endCycles = 0;
		m68kProfilerTableRecord[i].functionName = m68kProfilerTable[i].sourcefilename = nullptr;
		m68kProfilerTableRecord[i].callCount = 0;
		m68kProfilerTableRecord[i].minCycles = 0xffffffff;
		m68kProfilerTableRecord[i].maxCycles = 0;
	}
}


// Profiler 68000 new entry setup
void m68kProfilerEntryUp(unsigned int PCAdr)
{
	if (!M68KProfilerTableOverflow)
	{
		// check if address will be addressed later
		if (PCAdr == -1)
		{
			M68Kdeferentry = true;
		}
		else
		{
			// update the record table
			bool flag = false;
			for (int i = 0; (i < m68kProfilerEntryCountRecord) && !flag; i++)
			{
				if (m68kProfilerTableRecord[i].PCFuncAdr == PCAdr)
				{
					m68kProfilerTableRecord[i].callCount++;
					flag = true;
				}
			}
			if (!flag)
			{
				m68kProfilerTableRecord[m68kProfilerEntryCountRecord].PCFuncAdr = PCAdr;
				m68kProfilerTableRecord[m68kProfilerEntryCountRecord].functionName = DBGManager_GetSymbolNameFromAdr(PCAdr);
				m68kProfilerTableRecord[m68kProfilerEntryCountRecord].sourcefilename = DBGManager_GetFullSourceFilenameFromAdr(PCAdr, nullptr);
				m68kProfilerTableRecord[m68kProfilerEntryCountRecord].callCount = 1;
				++m68kProfilerEntryCountRecord;
			}

			// create a new entry in the current table
			m68kProfilerTable[++m68kProfilerEntryIndex].PCFuncAdr = PCAdr;
			m68kProfilerTable[m68kProfilerEntryIndex].functionName = DBGManager_GetSymbolNameFromAdr(PCAdr);
			m68kProfilerTable[m68kProfilerEntryIndex].sourcefilename = DBGManager_GetFullSourceFilenameFromAdr(PCAdr, nullptr);
			m68kProfilerTable[m68kProfilerEntryIndex].currentCycles = 0;
			m68kProfilerTable[m68kProfilerEntryIndex].startCycles = m68kProfilerTable[m68kProfilerEntryIndex].endCycles = M68KProfilerCycles;
			m68kProfilerTable[m68kProfilerEntryIndex].previousIndex = m68kProfilerEntryIndex - 1;
			// enter to the Tracy profiler
			m68kTracyProfiler->M68Kenter(&m68kProfilerTable[m68kProfilerEntryIndex].tracyCtx, m68kProfilerTable[m68kProfilerEntryIndex].functionName, m68kProfilerTable[m68kProfilerEntryIndex].sourcefilename, m68kProfilerTable[m68kProfilerEntryIndex].startCycles);
		}
	}
}


// Profiler 68000 current entry update
void m68kProfilerEntryUpdate(unsigned int PCAdr, unsigned int cycles)
{
	if (!M68KProfilerTableOverflow)
	{
		// check for deferred entry
		if (M68Kdeferentry)
		{
			// create a new entry
			m68kProfilerEntryUp(PCAdr);
			M68Kdeferentry = false;
		}

		// update the current cycles
		m68kProfilerTable[m68kProfilerEntryIndex].currentCycles += cycles;
		M68KProfilerCycles += cycles;
	}
}


// Profiler 68000 entry down
// PCAdr points on the RTS instruction
void m68kProfilerEntryDown(unsigned int PCAdr)
{
	if (!M68KProfilerTableOverflow)
	{
		// update the record table
		bool flag = false;
		for (int i = 0; (i < m68kProfilerEntryCountRecord) && !flag; i++)
		{
			if (m68kProfilerTableRecord[i].PCFuncAdr == m68kProfilerTable[m68kProfilerEntryIndex].PCFuncAdr)
			{
				// update min/max cycles
				if (m68kProfilerTableRecord[i].minCycles > m68kProfilerTable[m68kProfilerEntryIndex].currentCycles)
				{
					m68kProfilerTableRecord[i].minCycles = m68kProfilerTable[m68kProfilerEntryIndex].currentCycles;
				}

				if (m68kProfilerTableRecord[i].maxCycles < m68kProfilerTable[m68kProfilerEntryIndex].currentCycles)
				{
					m68kProfilerTableRecord[i].maxCycles = m68kProfilerTable[m68kProfilerEntryIndex].currentCycles;
				}

				flag = true;
			}
		}

		// update the number of cycles from the previous entry
		m68kProfilerTable[m68kProfilerTable[m68kProfilerEntryIndex].previousIndex].currentCycles += m68kProfilerTable[m68kProfilerEntryIndex].currentCycles;
		m68kProfilerTable[m68kProfilerEntryIndex].endCycles = M68KProfilerCycles;
		// leave the Tracy profiler
		m68kTracyProfiler->M68Kleave(&m68kProfilerTable[m68kProfilerEntryIndex].tracyCtx, m68kProfilerTable[m68kProfilerEntryIndex].currentCycles);

		// remove the current entry
		int index = m68kProfilerTable[m68kProfilerEntryIndex].previousIndex;
#ifdef _DEBUG
		m68kProfilerTable[m68kProfilerEntryIndex].previousIndex = -1;
		m68kProfilerTable[m68kProfilerEntryIndex].PCFuncAdr = 0;
		m68kProfilerTable[m68kProfilerEntryIndex].currentCycles = m68kProfilerTable[m68kProfilerEntryIndex].startCycles = m68kProfilerTable[m68kProfilerEntryIndex].endCycles = 0;
		m68kProfilerTable[m68kProfilerEntryIndex].functionName = m68kProfilerTable[m68kProfilerEntryIndex].sourcefilename = nullptr;
#endif
		// go back to previous entry
		m68kProfilerEntryIndex = index;
		// error check
		M68KProfilerTableOverflow = (m68kProfilerEntryIndex < 0) ? true : false;
	}
}


// Profiler flush and clear all profiling information
void ProfilerFlush(void)
{
	// flush all entries in Tracy profiler
	while (m68kProfilerEntryIndex >= 0)
	{
		m68kTracyProfiler->M68Kleave(&m68kProfilerTable[m68kProfilerEntryIndex].tracyCtx, m68kProfilerTable[m68kProfilerEntryIndex].currentCycles);
		m68kProfilerEntryIndex = m68kProfilerTable[m68kProfilerEntryIndex].previousIndex;
	}

	//
	ProfilerClear();
}


// Profiler reset
void ProfilerReset(void)
{
	ProfilerClear();
}
