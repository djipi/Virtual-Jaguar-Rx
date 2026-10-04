//
// vjrxprofiler.h: VJRx implementation of the profiler interface
//
// This class provides an implementation of the baseProfiler interface using the VJRx profiler (internal Virtual Jaguar Rx)
//
// Features:
// - M68000 CPU cycle tracking and function profiling
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  mm/dd/yyyy  What
// ---  ----------  -----------------------------------------------------------
// JPM   Dec./2025  Created this file
//

#ifndef __VJRXPROFILER_H__
#define __VJRXPROFILER_H__

#include <stdint.h>
#include "baseprofiler.h"
#include "profiler/vjrxprofilerwin.h"

    // Profiler type enumeration
enum class VJRxProfilerType_t : uint32_t { Null = 0, Record = 1 };

    // Profiler context structure for VJRx
struct VJRxZoneCtx {
    size_t id;
    bool active;
    VJRxProfilerType_t type;
};

//
class VJRxProfiler : public baseProfiler
{
private:
	// Maximum number of records for the VJRx profiler
    static constexpr size_t MAX_VJRXM68KRECORD = 10000;

public:
    VJRxProfiler(void);
    ~VJRxProfiler(void);
    //
    bool Start(void);
    void InitLua(lua_State*) override {}
    void Timer(bool, bool) override {}
    void Pause(bool);
    void RAZIndex(void* index);
    // Profiler 68000 functions
    void M68Kenter(void*, char*, char*, size_t, size_t) {}
    bool M68Kactive(void*) { return false; }
    void M68Kleave(void*, size_t) {}
    void M68Kmalloc(void*, size_t, size_t, int) {}
    void M68Kfree(void*, size_t, bool) {}
    void M68Krecord(void* index, char* functionName, char* filename, size_t linenumber, size_t callcount, size_t minCycles, size_t maxCycles);
    void M68KFrameStart(size_t) {}
    void M68KFrameEnd(size_t) {}

private:
    VJRxProfilerWindow* VJRxProfilerWin = nullptr;
    VJRxProfilerWindow::VJRxProfilerData M68KRecord[MAX_VJRXM68KRECORD];
    size_t NbM68KRecord;
    bool Paused;
};

#endif
