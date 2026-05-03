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
// JPM   Oct./2025       Added pause feature, flush data to the Tracy profiler and memory allocation tracking
// JPM   Nov./2025       Prepare code for multiple profilers, revamp the profiler initialization, frame loop
//

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "debugger/DBGManager.h"
#include "profiler.h"
#include "profiler/baseprofiler.h"
#include "profiler/dummyprofiler.h"
#include "profiler/tracyprofiler.h"
#include "profiler/vjrxprofiler.h"
#include "memory.h"


//
#define M68K_PROFILER_MAX_ENTRIES	200000

// Profiler type index
typedef enum { VJRXPROFILER_IDX, TRACYPROFILER_IDX } ProfilerIdx_t;
#define COUNT_PROFILERS	2


// Profiler context structure
// Structure must contains at least id, and active fields
union Ctx_s {
#ifdef TRACY_ENABLE
	TracyCZoneCtx TracyCtx;
#endif
	VJRxZoneCtx VJRxCtx;
};


// Structure to hold name's address in ASCII format
struct ProfilerNameEntry_s {
	size_t PCAdr;
	char Name[256];
};

// Structure to hold record profiling information
struct ProfilerRecordEntry_s {
	size_t PCFuncAdr;
	char* functionName;
	char* sourcefilename;
	size_t numline;
	size_t callCount;
	size_t minCycles;
	size_t maxCycles;
	Ctx_s Ctx[COUNT_PROFILERS];
};

// Structure to hold current profiling information
struct ProfilerCurrentEntry_s {
	int previousIndex;
	size_t PCFuncAdr;
	char* functionName;
	char* sourcefilename;
	size_t numline;
	size_t currentCycles;
	size_t startCycles;
	size_t endCycles;
	Ctx_s Ctx[COUNT_PROFILERS];
};

// Structure to hold memory profiling information
struct ProfilerMemoryRecord_s {
	size_t ptr;
	size_t size;
	Ctx_s Ctx[COUNT_PROFILERS];
};


// Arrays to hold profiling entries
ProfilerNameEntry_s m68kProfilerNamesTable[M68K_PROFILER_MAX_ENTRIES];
ProfilerCurrentEntry_s m68kProfilerTable[M68K_PROFILER_MAX_ENTRIES];
ProfilerRecordEntry_s m68kProfilerTableRecord[M68K_PROFILER_MAX_ENTRIES];
ProfilerMemoryRecord_s m68kProfilerMallocRecord[M68K_PROFILER_MAX_ENTRIES];
// Index and count
int m68kProfilerEntryIndex, m68kProfilerEntryCountRecord, m68kProfilerMallocIndex, m68kProfilerNamesCountRecord;
bool M68KProfilerTableOverflow;
// Specific to frame loop
S_FrameLoopInfo TableFrameLoopInfo;
// Specific to all profilers
int64_t g_total_cpu_cycles;
baseProfiler* BaseProfilers[COUNT_PROFILERS] = { nullptr };
//
bool M68Kdeferentry;


// Forward declarations
char* Profiler_RecordName(char* Name, size_t PCAdr);
void Profiler_ClearNames(void);
void Profiler_ClearRecord(void);
void Profiler_ClearCurrent(void);
void Profiler_ClearLoopFrameInfo(void);
void Profiler_Flush(void);


// Profiler initialization
void Profiler_Init(uint32_t type, lua_State* LuaLib)
{
	// VJ profiler setup
	(type & VJRXPROFILER) ? BaseProfilers[VJRXPROFILER_IDX] = new VJRxProfiler() : BaseProfilers[VJRXPROFILER_IDX] = new DummyProfiler();
	// Tracy profiler setup
#ifdef TRACY_ENABLE
	(type & TRACYPROFILER) ? BaseProfilers[TRACYPROFILER_IDX] = new TracyProfiler() : BaseProfilers[TRACYPROFILER_IDX] = new DummyProfiler();
#else
	BaseProfilers[TRACYPROFILER_IDX] = new DummyProfiler();
#endif
	// Profilers initialization
	for (size_t i = 0; i < COUNT_PROFILERS; i++)
	{
		BaseProfilers[i]->InitLua(LuaLib);
	}
}


// Start the profiler & the profilers
void Profiler_Start(void)
{
	// profiler agnostic initialization
	Profiler_ClearNames();
	Profiler_ClearRecord();
	Profiler_ClearCurrent();
	Profiler_ClearLoopFrameInfo();
	// reset total cycles
	g_total_cpu_cycles = 0;
	// Profilers initialization
	for (size_t i = 0; i < COUNT_PROFILERS; i++)
	{
		BaseProfilers[i]->Start();
	}
}


// Toggle on/off the profiler
void Profiler_Pause(bool onoff)
{
#if 0
	for (size_t i = 0; i < COUNT_PROFILERS; i++)
	{
		BaseProfilers[i]->Pause(onoff);
		//BaseProfilers[i]->Timer(onoff);
	}
#endif
}


// Toggle on/off the profiler type
// pause: true = resume, false = pause
void typeProfiler_Pause(bool pause, ProfilerType_t mode)
{
	switch (mode)
	{
	case TRACYPROFILER:
		BaseProfilers[TRACYPROFILER_IDX]->Pause(pause);
		break;

	case VJRXPROFILER:
		BaseProfilers[VJRXPROFILER_IDX]->Pause(pause);
		break;

	default:
		break;
	}
}


// Profiler names record initialization
void Profiler_ClearNames(void)
{
	// reset record count
	m68kProfilerNamesCountRecord = 0;
	// prepare the tables
	for (size_t i = 0; i < M68K_PROFILER_MAX_ENTRIES; i++)
	{
		m68kProfilerNamesTable[i].PCAdr = 0;
		m68kProfilerNamesTable[i].Name[0] = '\0';
	}
}


// Profiler record initialization
void Profiler_ClearRecord(void)
{
	// reset record count
	m68kProfilerEntryCountRecord = 0;
	// prepare the tables
	for (size_t i = 0; i < M68K_PROFILER_MAX_ENTRIES; i++)
	{
		//
		m68kProfilerTableRecord[i].PCFuncAdr = 0;
		m68kProfilerTableRecord[i].functionName = m68kProfilerTableRecord[i].sourcefilename = nullptr;
		m68kProfilerTableRecord[i].numline = 0;
		m68kProfilerTableRecord[i].callCount = 0;
		m68kProfilerTableRecord[i].minCycles = 0xffffffff;
		m68kProfilerTableRecord[i].maxCycles = 0;
		for (size_t j = 0; j < COUNT_PROFILERS; j++)
		{
			BaseProfilers[j]->RAZIndex((void*)&m68kProfilerTableRecord[i].Ctx[j]);
		}
	}
}


// Profiler current initialization
void Profiler_ClearCurrent(void)
{
	// set variables
	m68kProfilerEntryIndex = -1;
	m68kProfilerMallocIndex = 0;
	M68Kdeferentry = true;
	M68KProfilerTableOverflow = false;

	// prepare the tables
	for (size_t i = 0; i < M68K_PROFILER_MAX_ENTRIES; i++)
	{
		//
		m68kProfilerTable[i].previousIndex = -1;
		m68kProfilerTable[i].PCFuncAdr = 0;
		m68kProfilerTable[i].functionName = m68kProfilerTable[i].sourcefilename = nullptr;
		m68kProfilerTable[i].numline = 0;
		m68kProfilerTable[i].currentCycles = m68kProfilerTable[i].startCycles = m68kProfilerTable[i].endCycles = 0;
		for (size_t j = 0; j < COUNT_PROFILERS; j++)
		{
			BaseProfilers[j]->RAZIndex((void*)&m68kProfilerTable[i].Ctx[j]);
		}

		//
		m68kProfilerMallocRecord[i].ptr = m68kProfilerMallocRecord[i].size = 0;
		for (size_t k = 0; k < COUNT_PROFILERS; k++)
		{
			BaseProfilers[k]->RAZIndex((void*)&m68kProfilerMallocRecord[i].Ctx[k]);
		}
	}
}


// Profiler frame loop information initialization
void Profiler_ClearLoopFrameInfo(void)
{
	// clear frame loop information
	memset(&TableFrameLoopInfo, 0, sizeof(S_FrameLoopInfo));
}


// Profiler 68000 frame loop entry
bool m68kProfilerEntryLoopFrame(S_FrameLoopInfo* frameInfo)
{
	// keep the frame loop information
	memcpy(&TableFrameLoopInfo, frameInfo, sizeof(S_FrameLoopInfo));
	TableFrameLoopInfo.HitCounts = 0;
	TableFrameLoopInfo.Active = true;
	TableFrameLoopInfo.Used = false;
	// reset defer entry flag a loop frame cannot be deferred
	M68Kdeferentry = false;
	// success no matter what
	return true;
}


// Profiler 68000 new entry setup
// m68KSP holds the stack pointer
void m68kProfilerEntryUp(size_t PCAdr, size_t m68KSP)
{
	// update only if no overflow
	if (!M68KProfilerTableOverflow)
	{
		// check for frame loop
		if (TableFrameLoopInfo.Active && (PCAdr == TableFrameLoopInfo.Adr) && !TableFrameLoopInfo.Used)
		{
			TableFrameLoopInfo.Used = true;
			// notify profilers about frame start
			for (size_t j = 0; j < COUNT_PROFILERS; j++)
			{
				BaseProfilers[j]->M68KFrameStart(TableFrameLoopInfo.HitCounts);
			}
		}
		if (TableFrameLoopInfo.Used || !TableFrameLoopInfo.Active)
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
					m68kProfilerTableRecord[m68kProfilerEntryCountRecord].functionName = Profiler_RecordName(DBGManager_GetSymbolNameFromAdr(PCAdr), PCAdr);
					m68kProfilerTableRecord[m68kProfilerEntryCountRecord].sourcefilename = DBGManager_GetFullSourceFilenameFromAdr(PCAdr, nullptr);
					m68kProfilerTableRecord[m68kProfilerEntryCountRecord].numline = DBGManager_GetNumLineFromAdr(PCAdr, DBG_NO_TAG);
					m68kProfilerTableRecord[m68kProfilerEntryCountRecord].callCount = 1;
					++m68kProfilerEntryCountRecord;
				}

				// create a new entry in the current table
				m68kProfilerTable[++m68kProfilerEntryIndex].PCFuncAdr = PCAdr;
				m68kProfilerTable[m68kProfilerEntryIndex].functionName = Profiler_RecordName(DBGManager_GetSymbolNameFromAdr(PCAdr), PCAdr);
				m68kProfilerTable[m68kProfilerEntryIndex].sourcefilename = DBGManager_GetFullSourceFilenameFromAdr(PCAdr, nullptr);
				m68kProfilerTable[m68kProfilerEntryIndex].numline = DBGManager_GetNumLineFromAdr(PCAdr, DBG_NO_TAG);
				m68kProfilerTable[m68kProfilerEntryIndex].currentCycles = 0;
				m68kProfilerTable[m68kProfilerEntryIndex].startCycles = m68kProfilerTable[m68kProfilerEntryIndex].endCycles = g_total_cpu_cycles;
				m68kProfilerTable[m68kProfilerEntryIndex].previousIndex = m68kProfilerEntryIndex - 1;

				// enter function's name, or the address, to the profilers
				for (size_t j = 0; j < COUNT_PROFILERS; j++)
				{
					BaseProfilers[j]->M68Kenter(&m68kProfilerTable[m68kProfilerEntryIndex].Ctx[j], m68kProfilerTable[m68kProfilerEntryIndex].functionName, m68kProfilerTable[m68kProfilerEntryIndex].sourcefilename, m68kProfilerTable[m68kProfilerEntryIndex].numline, m68kProfilerTable[m68kProfilerEntryIndex].startCycles);
				}

				// check for memory allocation's name function
				if (!strncmp(m68kProfilerTable[m68kProfilerEntryIndex].functionName, "malloc", strlen("malloc")))
				{
					// get the memory allocation size parameter
					m68kProfilerMallocRecord[m68kProfilerMallocIndex].size = GET32(jagMemSpace, (m68KSP + 4));
				}
				else
				{
					// check for the free memory allocation's name function
					if (!strncmp(m68kProfilerTable[m68kProfilerEntryIndex].functionName, "free", strlen("free")))
					{
						// get the memory allocation pointer parameter
						size_t ptr = GET32(jagMemSpace, (m68KSP + 4));
						bool flag = false;
						if (ptr)
						{
							// look for the pointer in the memory allocation record
							for (size_t i = 0; (i < m68kProfilerMallocIndex) && !flag; i++)
							{
								if (m68kProfilerMallocRecord[i].ptr == ptr)
								{
									// ptr is found
									flag = true;
									// remove the memory allocation record from profilers
									for (size_t j = 0; j < COUNT_PROFILERS; j++)
									{
										BaseProfilers[j]->M68Kfree((void*)&m68kProfilerMallocRecord[i].Ctx[j], ptr, false);
									}
									// erase the pointer in the memory allocation record
									m68kProfilerMallocRecord[i].ptr = m68kProfilerMallocRecord[i].size = 0;
								}
							}

							if (!flag)
							{
								// update the memory allocation record in profilers with an unknown pointer
								for (size_t j = 0; j < COUNT_PROFILERS; j++)
								{
									BaseProfilers[j]->M68Kfree((void*)-1, ptr, false);
								}
							}
						}
						else
						{
							// update the memory allocation record in profilers with a null pointer
							for (size_t j = 0; j < COUNT_PROFILERS; j++)
							{
								BaseProfilers[j]->M68Kfree(nullptr, ptr, false);
							}
						}
					}
				}
			}
		}
	}
}


// Profiler 68000 current entry update
void m68kProfilerEntryUpdate(size_t PCAdr, size_t cycles, size_t m68KSP)
{
	// update only if no overflow
	if (!M68KProfilerTableOverflow)
	{
		// check for frame loop
		if (TableFrameLoopInfo.Active && (PCAdr == TableFrameLoopInfo.Adr) && !TableFrameLoopInfo.Used)
		{
			TableFrameLoopInfo.Used = true;
		}
		if (TableFrameLoopInfo.Used || !TableFrameLoopInfo.Active)
		{
			// check for deferred entry
			if (M68Kdeferentry)
			{
				// create a new entry
				m68kProfilerEntryUp(PCAdr, m68KSP);
				M68Kdeferentry = false;
			}

			// update the current cycles
			m68kProfilerTable[m68kProfilerEntryIndex].currentCycles += cycles;
			g_total_cpu_cycles += cycles;
		}
	}
}


// Profiler 68000 entry down
// PCAdr points on the RTS instruction
// m68KD0 holds the register used by function return value
void m68kProfilerEntryDown(size_t PCAdr, size_t m68KD0)
{
	// entry only if no overflow
	if (!M68KProfilerTableOverflow)
	{
		// check for frame loop
		if (TableFrameLoopInfo.Used || !TableFrameLoopInfo.Active)
		{
			// frame loop hit count update
			if (TableFrameLoopInfo.Used && (m68kProfilerTable[m68kProfilerEntryIndex].PCFuncAdr == TableFrameLoopInfo.Adr))
			{
				// notify profilers about frame end
				for (size_t j = 0; j < COUNT_PROFILERS; j++)
				{
					BaseProfilers[j]->M68KFrameEnd(TableFrameLoopInfo.HitCounts);
				}
				// update hit counts
				TableFrameLoopInfo.HitCounts++;
				TableFrameLoopInfo.Used = false;
			}

			// update the record table
			bool flag = false;
			for (size_t i = 0; (i < m68kProfilerEntryCountRecord) && !flag; i++)
			{
				if (m68kProfilerTableRecord[i].PCFuncAdr == m68kProfilerTable[m68kProfilerEntryIndex].PCFuncAdr)
				{
					// update minimum cycles
					if (m68kProfilerTableRecord[i].minCycles > m68kProfilerTable[m68kProfilerEntryIndex].currentCycles)
					{
						m68kProfilerTableRecord[i].minCycles = m68kProfilerTable[m68kProfilerEntryIndex].currentCycles;
					}
					// update maximum cycles
					if (m68kProfilerTableRecord[i].maxCycles < m68kProfilerTable[m68kProfilerEntryIndex].currentCycles)
					{
						m68kProfilerTableRecord[i].maxCycles = m68kProfilerTable[m68kProfilerEntryIndex].currentCycles;
					}

					//
					for (size_t j = 0; j < COUNT_PROFILERS; j++)
					{
						BaseProfilers[j]->M68Krecord((void*)&m68kProfilerTableRecord[i].Ctx[j], m68kProfilerTableRecord[i].functionName, m68kProfilerTableRecord[i].callCount, m68kProfilerTableRecord[i].minCycles, m68kProfilerTableRecord[i].maxCycles);
					}

					flag = true;
				}
			}

			// update the number of cycles from the previous entry
			m68kProfilerTable[m68kProfilerTable[m68kProfilerEntryIndex].previousIndex].currentCycles += m68kProfilerTable[m68kProfilerEntryIndex].currentCycles;
			m68kProfilerTable[m68kProfilerEntryIndex].endCycles = g_total_cpu_cycles;
			// leave the profilers
			for (size_t j = 0; j < COUNT_PROFILERS; j++)
			{
				BaseProfilers[j]->M68Kleave((void*)&m68kProfilerTable[m68kProfilerEntryIndex].Ctx[j], m68kProfilerTable[m68kProfilerEntryIndex].currentCycles);
			}

			// check for the memory allocation's name function
			if (!strncmp(m68kProfilerTable[m68kProfilerEntryIndex].functionName, "malloc", strlen("malloc")))
			{
				// add the memory allocation record in the profilers
				m68kProfilerMallocRecord[m68kProfilerMallocIndex].ptr = m68KD0;
				for (size_t j = 0; j < COUNT_PROFILERS; j++)
				{
					BaseProfilers[j]->M68Kmalloc((void*)&m68kProfilerMallocRecord[m68kProfilerMallocIndex].Ctx[j], m68KD0, m68kProfilerMallocRecord[m68kProfilerMallocIndex].size, m68kProfilerMallocIndex);
				}
				m68kProfilerMallocIndex++;
			}

#if 1
			// remove the current entry
			int index = m68kProfilerTable[m68kProfilerEntryIndex].previousIndex;
			m68kProfilerTable[m68kProfilerEntryIndex].previousIndex = -1;
			m68kProfilerTable[m68kProfilerEntryIndex].PCFuncAdr = 0;
			m68kProfilerTable[m68kProfilerEntryIndex].functionName = m68kProfilerTable[m68kProfilerEntryIndex].sourcefilename = nullptr;
			m68kProfilerTable[m68kProfilerEntryIndex].numline = 0;
			m68kProfilerTable[m68kProfilerEntryIndex].currentCycles = m68kProfilerTable[m68kProfilerEntryIndex].startCycles = m68kProfilerTable[m68kProfilerEntryIndex].endCycles = 0;
			for (size_t j = 0; j < COUNT_PROFILERS; j++)
			{
				BaseProfilers[j]->RAZIndex((void*)&m68kProfilerTable[m68kProfilerEntryIndex].Ctx[j]);
			}
			// go back to previous entry
			m68kProfilerEntryIndex = index;
#else
			m68kProfilerEntryIndex = m68kProfilerTable[m68kProfilerEntryIndex].previousIndex;
#endif
			// error check
			M68KProfilerTableOverflow = (!TableFrameLoopInfo.Active && (m68kProfilerEntryIndex < 0)) ? true : false;
		}
	}
}


// Profiler flush, and clear, all profiling information
void Profiler_Flush(void)
{
	// flush all memory allocation records from profilers
	while (--m68kProfilerMallocIndex >= 0)
	{
		for (size_t i = 0; i < COUNT_PROFILERS; i++)
		{
			do
			{
				BaseProfilers[i]->M68Kfree((void*)&m68kProfilerMallocRecord[m68kProfilerMallocIndex].Ctx[i], m68kProfilerMallocRecord[m68kProfilerMallocIndex].ptr, true);
			} while (BaseProfilers[i]->M68Kactive((void*)&m68kProfilerMallocRecord[m68kProfilerMallocIndex].Ctx[i]));
		}
	}

	// flush all entries in profilers
	while (--m68kProfilerEntryIndex >= 0)
	{
		for (size_t i = 0; i < COUNT_PROFILERS; i++)
		{
			do
			{
				BaseProfilers[i]->M68Kleave((void*)&m68kProfilerTable[m68kProfilerEntryIndex].Ctx[i], m68kProfilerTable[m68kProfilerEntryIndex].currentCycles);
			} while (BaseProfilers[i]->M68Kactive((void*)&m68kProfilerTable[m68kProfilerEntryIndex].Ctx[i]));
		}
	}
}


// Profilers reset
void Profiler_Reset(void)
{
	// flush data, from the profiler, to the profilers
	Profiler_Flush();
	// clear profilers for a new round of profiling
	Profiler_ClearCurrent();
	Profiler_ClearRecord();
	Profiler_ClearLoopFrameInfo();
	// reset total cycles
	g_total_cpu_cycles = 0;
	// reinitialize timer for each profiler
	for (size_t i = 0; i < COUNT_PROFILERS; i++)
	{
		BaseProfilers[i]->Timer(false, true);
	}
}


// Record name based on value
char* Profiler_RecordName(char* Name, size_t PCAdr)
{
	if (!Name)
	{
		// look for the name in the record table
		for (size_t k = 0; (k < m68kProfilerNamesCountRecord); k++)
		{
			if (m68kProfilerNamesTable[k].PCAdr == PCAdr)
			{
				// function address is found
				return m68kProfilerNamesTable[k].Name;
			}
		}

		// add the name to the record table
		m68kProfilerNamesTable[m68kProfilerNamesCountRecord].PCAdr = PCAdr;
		// set name based on the address's value
		sprintf(m68kProfilerNamesTable[m68kProfilerNamesCountRecord].Name, "$%06x", (unsigned int)PCAdr);
		return m68kProfilerNamesTable[m68kProfilerNamesCountRecord++].Name;
	}
	else
	{
		return Name;
	}
}


// Profilers closing
void Profiler_Close(void)
{
	for (size_t i = 0; i < COUNT_PROFILERS; i++)
	{
		delete BaseProfilers[i];
		BaseProfilers[i] = nullptr;
	}
}
