//
// log.h: Logfile support
//

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#if 0
#ifdef __cplusplus
extern "C" {
#endif
#endif

extern int LogInit(const char *);
extern FILE * LogGet(void);
extern void LogDone(void);
extern void WriteLog(const char * text, ...);

#if 0
#ifdef __cplusplus
}
#endif
#endif

// Some useful defines... :-) but not used
//#define GPU_DEBUG
//#define LOG_BLITS

#endif	// __LOG_H__
