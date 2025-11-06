//
// profiler.h: Virtual Jaguar Rx profiler subsystem interface
//
// This is the main profiler interface for Virtual Jaguar Rx, providing an unified API for performance analysis and profiling capabilities.
//
// Features:
// - Multiple profiler backend support
// - M68000 CPU profiling with function entry/exit tracking
// - Lua scripting integration for custom profiling workflows
// - Configurable pause/resume and reset capabilities
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM   Nov./2025       Created this file
//

#ifndef __PROFILER_H__
#define __PROFILER_H__

#include <stdint.h>
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

//
typedef enum {	VJPROFILER,	TRACYPROFILER, 
				COUNT_PROFILERS } ProfilerType_t;

// Profiler generic functions
extern void Profiler_Init(lua_State* LuaLib);
extern void profiler_Start(void);
extern void Profiler_Pause(bool onoff);
extern void Profiler_Reset(void);
extern void Profiler_Close(void);
// Profiler specific types
extern void typeProfiler_Pause(bool pause, ProfilerType_t mode);
// Profiler 68000 functions
extern void m68kProfilerEntryUp(size_t PCAdr, size_t m68KSP);
extern void m68kProfilerEntryDown(size_t PCAdr, size_t m68KD0);
extern void m68kProfilerEntryUpdate(size_t PCAdr, size_t cycles, size_t m68KSP);

#endif
