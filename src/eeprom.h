//
// EEPROM.H: Header file
//

#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <stdint.h>

extern void EepromInit(void);
extern void EepromReset(void);
extern void EepromDone(void);

extern uint8_t EepromReadByte(uint32_t offset);
extern uint16_t EepromReadWord(uint32_t offset);
extern void EepromWriteByte(uint32_t offset, uint8_t data);
extern void EepromWriteWord(uint32_t offset, uint16_t data);

#endif	// __EEPROM_H__
