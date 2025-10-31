#ifndef __PROFILER_H__
#define __PROFILER_H__

extern void ProfilerInit(void);
extern void ProfilerPause(bool pause);
extern void ProfilerFlush(void);
extern void m68kProfilerEntryUp(unsigned int PCAdr, unsigned int m68KSP);
extern void m68kProfilerEntryDown(unsigned int PCAdr, unsigned int m68KD0);
extern void m68kProfilerEntryUpdate(unsigned int PCAdr, unsigned int cycles, unsigned int m68KSP);
extern void ProfilerReset(void);

#endif
