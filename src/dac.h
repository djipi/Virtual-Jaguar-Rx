//
// DAC.H: Header file
//

#ifndef __DAC_H__
#define __DAC_H__

// DAC defines
#define SMODE_INTERNAL		0x01
#define SMODE_MODE			0x02
#define SMODE_WSEN			0x04
#define SMODE_RISING		0x08
#define SMODE_FALLING		0x10
#define SMODE_EVERYWORD		0x20

// General functions
extern void DACInit(void);
extern void DACReset(void);
extern void DACPauseAudioThread(bool state = true);
extern void DACDone(void);

// Savestate functions
extern uint32_t DacReadSavestate(unsigned char *ptrsst);
extern uint32_t DacWriteSavestate(unsigned char *ptrsst);

// DAC memory access
extern void DACWriteByte(uint32_t offset, uint8_t data, uint32_t who = UNKNOWN);
extern void DACWriteWord(uint32_t offset, uint16_t data, uint32_t who = UNKNOWN);
extern uint8_t DACReadByte(uint32_t offset, uint32_t who = UNKNOWN);
extern uint16_t DACReadWord(uint32_t offset, uint32_t who = UNKNOWN);

#endif	// __DAC_H__
