//
// Jaguar blitter implementation
//

#ifndef __BLITTER_H__
#define __BLITTER_H__

// General functions
extern void BlitterInit(void);
extern void BlitterReset(void);
extern void BlitterDone(void);

// Savestate functions
extern uint32_t BlitterReadSavestate(unsigned char *ptrsst);
extern uint32_t BlitterWriteSavestate(unsigned char *ptrsst);

// Blitter memory access
extern uint8_t BlitterReadByte(uint32_t, uint32_t who = UNKNOWN);
extern uint16_t BlitterReadWord(uint32_t, uint32_t who = UNKNOWN);
extern uint32_t BlitterReadLong(uint32_t, uint32_t who = UNKNOWN);
extern void BlitterWriteByte(uint32_t, uint8_t, uint32_t who = UNKNOWN);
extern void BlitterWriteWord(uint32_t, uint16_t, uint32_t who = UNKNOWN);
extern void BlitterWriteLong(uint32_t, uint32_t, uint32_t who = UNKNOWN);

//For testing only...
extern void LogBlit(void);

#endif	// __BLITTER_H__
