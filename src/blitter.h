//
// Jaguar blitter implementation
//

#ifndef __BLITTER_H__
#define __BLITTER_H__

//#include "types.h"
#include "memory.h"

typedef struct Blitter_s
{
	//bool available;
	uint32_t A1_adr;
	uint32_t A1_pitch;
	uint32_t A1_pixelsize;
} Blitter_s;

extern void BlitterInit(void);
extern void BlitterReset(void);
extern void BlitterDone(void);
extern Blitter_s* BlitterGetDefsReg(Blitter_s* BlitDes);
extern uint8_t BlitterReadByte(uint32_t, uint32_t who = UNKNOWN);
extern uint16_t BlitterReadWord(uint32_t, uint32_t who = UNKNOWN);
extern uint32_t BlitterReadLong(uint32_t, uint32_t who = UNKNOWN);
extern void BlitterWriteByte(uint32_t, uint8_t, uint32_t who = UNKNOWN);
extern void BlitterWriteWord(uint32_t, uint16_t, uint32_t who = UNKNOWN);
extern void BlitterWriteLong(uint32_t, uint32_t, uint32_t who = UNKNOWN);
//uint32_t blitter_reg_read(uint32_t offset);
//void blitter_reg_write(uint32_t offset, uint32_t data);

extern bool blitter_working;
extern bool bpbActive;

//For testing only...
//void LogBlit(void);

#endif	// __BLITTER_H__
