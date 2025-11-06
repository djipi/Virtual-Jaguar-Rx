//
// baseprofiler.h: Abstract base class for Virtual Jaguar Rx profiler implementations
//
// This is the abstract interface that all profiler implementations must inherit from and implement.
// It defines the contract for profiling M68000 CPU operations, memory allocations, and integration with the Lua scripting system.
//
// Features:
// - M68000 function profiling (enter/leave with cycle counting)
// - Memory allocation tracking (malloc/free operations)
// - Lua scripting interface for custom profiling workflows
// - Timer control and pause/resume functionality
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM   Nov./2025       Created this file
//

#ifndef __BASEPROFILER_H__
#define __BASEPROFILER_H__

#include <stdint.h>
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

class baseProfiler
{
public:
	virtual bool Start(void) = 0;
	virtual void InitLua(lua_State* LuaLib) = 0;
    virtual void Timer(bool onoff, bool newvalue) = 0;
    virtual void Pause(bool pause) = 0;
	virtual void RAZIndex(void* index) = 0;
	virtual void M68Kenter(void* zoneCtx, char* functionName, char* filename, size_t linenumber, size_t startCycle) = 0;
	virtual bool M68Kactive(void* zoneCtx) = 0;
	virtual void M68Kleave(void* zoneCtx, size_t usedCycles) = 0;
	virtual void M68Kmalloc(void* zoneCtx, size_t ptr, size_t size, int depth) = 0;
	virtual void M68Kfree(void* zoneCtx, size_t ptr, bool flush) = 0;
    virtual ~baseProfiler() = default;
};

#endif
