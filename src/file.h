//
// FILE.H
//
// File support
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  06/15/2016  ELF format support
// JPM  06/19/2016  Soft debugger support
//

#ifndef __FILE_H__
#define __FILE_H__

#include <stdint.h>

#if 0
#ifdef __cplusplus
extern "C" {
#endif
#endif

enum FileType { FT_SOFTWARE=0, FT_EEPROM, FT_LABEL, FT_BOXART, FT_OVERLAY };
// JST = Jaguar Software Type
enum { JST_NONE = 0, JST_ROM, JST_ALPINE, JST_ABS_TYPE1, JST_ABS_TYPE2, JST_JAGSERVER, JST_WTFOMGBBQ, JST_ELF32 };

extern uint32_t JaguarLoadROM(uint8_t * &rom, char * path);
extern bool JaguarLoadFile(char * path);
extern bool AlpineLoadFile(char * path);
extern bool DebuggerLoadFile(char * path);
extern uint32_t GetFileFromZIP(const char * zipFile, FileType type, uint8_t * &buffer);
extern uint32_t GetFileDBIdentityFromZIP(const char * zipFile);
extern bool FindFileInZIPWithCRC32(const char * zipFile, uint32_t crc);
extern uint32_t ParseFileType(uint8_t * buffer, uint32_t size);
extern bool HasUniversalHeader(uint8_t * rom, uint32_t romSize);

#if 0
#ifdef __cplusplus
}
#endif
#endif

#endif	// __FILE_H__
