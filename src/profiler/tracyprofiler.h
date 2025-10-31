//
// tracyprofiler.h: Profiler main header with Tracy
//
// by Jean-Paul Mari
//

#ifndef __TRACYPROFILER_H__
#define __TRACYPROFILER_H__

#ifndef TRACY_ENABLE
#error "Tracy must be compiled statically with TRACY_ENABLE defined"
#else
#include "tracy\TracyC.h"
//#include "tracy\tracy.hpp"
#endif

class TracyProfiler
{
public:
	TracyProfiler(void);
	void Pause(bool pause);
	char* IntegerToStringWithCommas(char* out, unsigned int len, unsigned int value);
	void M68Kenter(TracyCZoneCtx* zoneCtx, char* functionName, char* filename, unsigned int linenumber, unsigned int startCycle);
	void M68Kleave(TracyCZoneCtx* pZone, unsigned int usedCycles);
	void M68Kmalloc(TracyCZoneCtx* zoneCtx, unsigned int ptr, unsigned int size, int depth);
	void M68Kfree(TracyCZoneCtx* zoneCtx, unsigned int ptr);
	~TracyProfiler(void);

private:
	bool tracyPaused;
};

#endif
