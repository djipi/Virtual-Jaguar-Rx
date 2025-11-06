//
// dummyprofiler.h: Null object implementation of the profiler interface
//
// This class provides a no-op (do-nothing) implementation of the baseProfiler interface, following the Null Object design pattern.
// It allows the profiling subsystem to be disabled at runtime without requiring conditional checks throughout the codebase.
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM   Nov./2025       Created this file
//

#ifndef __DUMMYPROFILER_H__
#define __DUMMYPROFILER_H__

#include <stdint.h>
#include "baseprofiler.h"

class DummyProfiler : public baseProfiler
{
public:
    bool Start(void) override { return false; }
    void InitLua(lua_State*) override {}
    void Timer(bool, bool) override {}
    void Pause(bool) override {}
	void RAZIndex(void*) {}
    void M68Kenter(void*, char*, char*, size_t, size_t) override {}
    bool M68Kactive(void*) override { return false; }
    void M68Kleave(void*, size_t) override {}
    void M68Kmalloc(void*, size_t, size_t, int) override {}
    void M68Kfree(void*, size_t, bool) override {}
};

#endif
