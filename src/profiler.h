//
// profiler.h: Virtual Jaguar Rx profiler subsystem interface
//
// This is the main profiler interface for Virtual Jaguar Rx, providing an unified API for performance analysis and profiling capabilities.
//
// Features:
// - Multiple profiler back-end support
// - M68000 CPU profiling with function entry/exit tracking
// - Lua scripting integration for custom profiling workflows
// - Configurable pause/resume and reset capabilities
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  mm/dd/yyyy  What
// ---  ----------  -----------------------------------------------------------
// JPM   Nov./2025  Created this file
// JPM  06/09/2026  Make Lua optional
//

#ifndef __PROFILER_H__
#define __PROFILER_H__

#include <stdint.h>
#ifdef LUA_ENABLE
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}
#else
typedef void lua_State;
#define luaL_newstate()	(lua_State*)-1
#define luaL_openlibs(LuaLib)
#define lua_close(LuaLib)
#endif

// Frame loop structure
typedef struct FrameLoopInfo
{
	bool Used;				// Allocated frame loop
	bool Active;			// Active frame loop
	char* Name;				// Functions's name
	char* Filename;			// Source filename
	char* LineSrc;			// Source code line
	size_t NumLine;			// Line number
	size_t Adr;				// Frame loop address
	size_t HitCounts;		// Hit counts
} S_FrameLoopInfo;

// Profiler types list (must be binary based)
typedef enum {	NOPROFILER = 0x0, VJRXPROFILER = 0x1, TRACYPROFILER = 0x2	} ProfilerType_t;

// Profiler generic functions
extern void Profiler_Init(uint32_t type, lua_State* LuaLib);
extern void Profiler_Start(void);
extern void Profiler_Pause(bool onoff);
extern void Profiler_Reset(void);
extern void Profiler_Close(void);
// Profiler specific types
extern void typeProfiler_Pause(bool pause, ProfilerType_t mode);
// Profiler 68000 functions
extern void m68kProfilerEntryUp(size_t PCAdr, size_t m68KSP);
extern void m68kProfilerEntryDown(size_t PCAdr, size_t m68KD0);
extern void m68kProfilerEntryUpdate(size_t PCAdr, size_t cycles, size_t m68KSP);
extern bool m68kProfilerEntryLoopFrame(S_FrameLoopInfo* frameInfo);

#endif
