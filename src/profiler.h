#ifndef __PROFILER_H__
#define __PROFILER_H__

extern void ProfilerInit(void);
extern void ProfilerPause(bool pause);
extern void ProfilerFlush(void);
extern void m68kProfilerEntryUp(unsigned int PCAdr);
extern void m68kProfilerEntryDown(unsigned int PCAdr);
extern void m68kProfilerEntryUpdate(unsigned int PCAdr, unsigned int cycles);
extern void ProfilerReset(void);

#endif
