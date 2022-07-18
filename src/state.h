//
// state.h: Machine state save/load support
//
// by James L. Hammons
//
// Patches
// https://atariage.com/forums/topic/243174-save-states-for-virtual-jaguar-patch/
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//  PL = PvtLewis <from Atari Age>
//
// Who  When        What
// JPM  March/2022  Added, and modified, the save state patch from PvtLewis
//

#ifndef __STATE_H__
#define __STATE_H__

#include <stdint.h>
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

#define SAVESTATEPATCH_PvtLewis


// macros to store stuff in machine independent formats

//#include <arpa/inet.h>
//#include <netinet/in.h>

#define SWAP16(_x)	(((_x & 0xff00) >> 8) | ((_x & 0x00ff) << 8))
#define	SWAP32(_x)	(((_x & 0xff000000) >> 24) | ((_x & 0x00ff0000) >> 8) | ((_x & 0x0000ff00) << 8) | ((_x & 0x000000ff) << 24))

inline uint64_t htonll(uint64_t value)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	const uint32_t high_part = SWAP32((uint32_t)(value >> 32));
	const uint32_t low_part = SWAP32((uint32_t)(value & 0xFFFFFFFFLL));
	return ((uint64_t)(((uint64_t)low_part << 32) | high_part));
#else
	return value;
#endif
}

#define ntohll(x) htonll(x)

extern size_t DumpSaveState(void);
extern size_t LoadSaveState(void);
extern size_t CanTryToLoadSaveState(void);

#define DUMP(_x) do { if (fwrite(&_x, sizeof(_x), 1, fp) != 1) { /* WriteLog("SaveState DUMP error at %s:%d\n", __FILE__, __LINE__); */ return -1; } total_dumped += sizeof(_x); } while (0)
#define DUMPBYTES(_x, _len) do { int _r; _r = fwrite(_x, 1, _len, fp); if (_r != _len) { /* WriteLog("SaveState DUMP error at %s:%d: expected %d got %d\n", __FILE__, __LINE__, _len, _r); */ return -1; } total_dumped += _len; } while (0)
#define LOAD(_x) do { if (fread(&_x, sizeof(_x), 1, fp) != 1) { /* WriteLog("SaveState LOAD error at %s:%d\n", __FILE__, __LINE__); */ return -1; } total_loaded += sizeof(_x); } while (0)
#define LOADBYTES(_x, _len) do { int _r; _r = fread(_x, 1, _len, fp); if (_r != _len) { /* WriteLog("SaveState LOAD error at %s:%d: expected %d got %d\n", __FILE__, __LINE__, _len, _r); */ return -1; } total_loaded += _len; } while (0)

#define HTOF16(x) (SWAP16(x))
#define FTOH16(x) (SWAP16(x))
#define HTOF32(x) (SWAP32(x))
#define FTOH32(x) (SWAP32(x))
#define HTOF64(x) (htonll(x))
#define FTOH64(x) (ntohll(x))

#define DUMP8(_z) DUMP(_z)
#define LOAD8(_z) LOAD(_z)

#define DUMP16(_z) do { uint16_t _d_tmp_u16 = HTOF16(_z); DUMP(_d_tmp_u16); } while (0)
#define DUMP32(_z) do { uint32_t _d_tmp_u32 = HTOF32(_z); DUMP(_d_tmp_u32); } while (0)
#define DUMP64(_z) do { uint64_t _d_tmp_u64 = HTOF64(_z); DUMP(_d_tmp_u64); } while (0)
#define DUMPBOOL(_z) do { uint32_t _tmp_u32 = _z ? 1 : 0; DUMP32(_tmp_u32); /*WriteLog("dumped bool %u at %ld\n", _tmp_u32, total_dumped-4);*/ } while (0)
#define DUMPPSTR(_pz) do { int _strlen = strlen(_pz); if (_strlen > 255) _strlen = 255; uint8_t _len8 = _strlen & 0xff; DUMP8(_len8); DUMPBYTES(_pz, _len8); } while (0)
#define DUMPDOUBLE(_z) do { char _tmp_buf[256]; sprintf(_tmp_buf, "%f", _z); DUMPPSTR(_tmp_buf); /*WriteLog("dumped double %f at %s:%d\n", _z, __FILE__, __LINE__);*/ } while (0)
#define LOAD16(_z) do { uint16_t _l_tmp_u16; LOAD(_l_tmp_u16); _l_tmp_u16 = FTOH16(_l_tmp_u16); memcpy(&_z, &_l_tmp_u16, 2); } while (0)
#define LOAD32(_z) do { uint32_t _l_tmp_u32; LOAD(_l_tmp_u32); _l_tmp_u32 = FTOH32(_l_tmp_u32); memcpy(&_z, &_l_tmp_u32, 4); } while (0)
#define LOAD64(_z) do { uint64_t _l_tmp_u64; LOAD(_l_tmp_u64); _l_tmp_u64 = FTOH64(_l_tmp_u64); memcpy(&_z, &_l_tmp_u64, 8); } while (0)
#define LOADBOOL(_z) do { uint32_t _tmp_u32; LOAD32(_tmp_u32); /*WriteLog("loaded bool %u at %ld\n", _tmp_u32, total_loaded-4);*/_z = _tmp_u32 == 1 ? true : false; } while (0)
#define LOADPSTR(_pz) do { uint8_t _len8; LOAD8(_len8); LOADBYTES(_pz, _len8); _pz[_len8] = '\0'; } while (0)
#define LOADDOUBLE(_z) do { char _tmp_buf[256]; LOADPSTR(_tmp_buf); _z = strtod(_tmp_buf, NULL); /*WriteLog("loaded double %f at %s:%d\n", _z, __FILE__, __LINE__);*/ } while (0)

#define LOADINT(_z) LOAD32(_z)
#define DUMPINT(_z) DUMP32(_z)
#define LOADUINT(_z) LOAD32(_z)
#define DUMPUINT(_z) DUMP32(_z)

#define DUMPS32(_z) DUMP32(_z)
#define LOADS32(_z) LOAD32(_z)

#define DUMPARR(_x) do { if (fwrite(_x, sizeof(_x), 1, fp) != 1) { /* WriteLog("DUMP error at %s:%d\n", __FILE__, __LINE__); */ return -1; } total_dumped += sizeof(_x); } while (0)
#define LOADARR(_x) do { if (fread(_x, sizeof(_x), 1, fp) != 1) { /* WriteLog("LOAD error at %s:%d\n", __FILE__, __LINE__); */ return -1; } total_loaded += sizeof(_x); } while (0)

#define DUMPARR8(_z) DUMPBYTES(_z, sizeof(_z))
#define LOADARR8(_z) LOADBYTES(_z, sizeof(_z))
#define DUMPARR16(_z) do { uint16_t _tmp_arr[sizeof(_z)/sizeof(_z[0])]; for (int _arr_idx = 0; _arr_idx < sizeof(_z)/sizeof(_z[0]); _arr_idx++) { _tmp_arr[_arr_idx] = HTOF16(_z[_arr_idx]); } DUMPARR(_tmp_arr); } while (0)
#define LOADARR16(_z) do { LOADARR(_z); for (int _arr_idx = 0; _arr_idx < sizeof(_z)/sizeof(_z[0]); _arr_idx++) { _z[_arr_idx] = FTOH16(_z[_arr_idx]); } } while (0)
#define DUMPARR32(_z) do { uint32_t _tmp_arr[sizeof(_z)/sizeof(_z[0])]; for (int _arr_idx = 0; _arr_idx < sizeof(_z)/sizeof(_z[0]); _arr_idx++) { _tmp_arr[_arr_idx] = HTOF32(_z[_arr_idx]); } DUMPARR(_tmp_arr); } while (0)
#define LOADARR32(_z) do { LOADARR(_z); for (int _arr_idx = 0; _arr_idx < sizeof(_z)/sizeof(_z[0]); _arr_idx++) { _z[_arr_idx] = FTOH32(_z[_arr_idx]); } } while (0)

// dump/load functions for each subsystem

#ifdef __cplusplus
extern "C" {
#endif
extern size_t gpu_dump (FILE *);
extern size_t gpu_load (FILE *);
extern size_t dsp_dump (FILE *);
extern size_t dsp_load (FILE *);
extern size_t tom_dump (FILE *);
extern size_t tom_load (FILE *);
extern size_t jerry_dump (FILE *);
extern size_t jerry_load (FILE *);
extern size_t op_dump (FILE *);
extern size_t op_load (FILE *);
extern size_t blitter_dump (FILE *fp);
extern size_t blitter_load (FILE *fp);
extern size_t dac_dump (FILE *fp);
extern size_t dac_load (FILE *fp);
extern size_t events_dump (FILE *fp);
extern size_t events_load (FILE *fp);
extern size_t m68k_dump (FILE *fp);
extern size_t m68k_load (FILE *fp);
extern size_t eeprom_dump (FILE *fp);
extern size_t eeprom_load (FILE *fp);
extern size_t eeprom2_dump (FILE *fp);
extern size_t eeprom2_load (FILE *fp);
extern size_t joystick_dump (FILE *fp);
extern size_t joystick_load (FILE *fp);
extern size_t cdrom_dump (FILE *fp);
extern size_t cdrom_load (FILE *fp);
#ifdef __cplusplus
}
#endif
#if 0
extern bool SaveState(void);
extern bool LoadState(void);
#endif


#endif	// __STATE_H__
