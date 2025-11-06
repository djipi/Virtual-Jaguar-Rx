//
// tracyprofiler.h: Tracy profiler integration for Virtual Jaguar Rx
//
// This class provides an implementation of the baseProfiler interface using the Tracy profiler (https://github.com/wolfpld/tracy)
// It supports real-time performance analysis and visualization.
//
// Features:
// - M68000 CPU cycle tracking and function profiling
// - Memory allocation tracking (malloc/free)
// - Lua integration support
// - Real-time connection to Tracy server
//
// Note: This header is only active when TRACY_ENABLE is defined.
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM   Nov./2025       Created this file
//

#if !defined(__TRACYPROFILER_H__) && defined(TRACY_ENABLE)
#define __TRACYPROFILER_H__

#include <stdint.h>
#include "baseprofiler.h"
//#include "tracy/Tracy.hpp"
#include "tracy\TracyC.h"

class TracyProfiler : public baseProfiler
{
public:
	TracyProfiler(void);
	bool Start(void) override;
	void InitLua(lua_State* LuaLib) override;
	void Timer(bool onoff, bool newvalue) override;
	void Pause(bool pause) override;
	void RAZIndex(void* index) override;
	void M68Kenter(void* zoneCtx, char* functionName, char* filename, size_t linenumber, size_t startCycle) override;
	bool M68Kactive(void* zoneCtx) override;
	void M68Kleave(void* zoneCtx, size_t usedCycles) override;
	void M68Kmalloc(void* zoneCtx, size_t ptr, size_t size, int depth) override;
	void M68Kfree(void* zoneCtx, size_t ptr, bool flush) override;
	~TracyProfiler(void);

private:
	char* IntegerToStringWithCommas(char* out, size_t len, size_t value);
	bool WaitForConnection(void);

private:
	bool tracyPaused;
};

#endif
