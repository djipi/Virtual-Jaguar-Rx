//
// state.cpp: VJ machine state save/load support
//
// by James Hammons
// (C) 2010 Underground Software
//
// Patches
// https://atariage.com/forums/topic/243174-save-states-for-virtual-jaguar-patch/
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//  PL = PvtLewis <from Atari Age>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/16/2010  Created this log ;-)
// JPM  March/2022  Added the save state patch from PvtLewis
//

#include "jaguar.h"
#include "SDL_opengl.h"
#include "blitter.h"
#include "cdrom.h"
#include "dac.h"
#include "dsp.h"
#include "eeprom.h"
#include "event.h"
#include "foooked.h"
#include "gpu.h"
#include "jerry.h"
#include "joystick.h"
#include "log.h"
//#include "mmu.h"
#include "settings.h"
#include "tom.h"
#include "state.h"
#include <string.h>
#include <errno.h>
#include <zlib.h>
#include "m68000/m68kinterface.h"
#include "m68000/cpudefs.h"


// Save states

// Save state file format:
// +--------------------------------------------------+
// | 4 byte file header | VJ <flags> <version number> |
// +--------------------------------------------------+
// First two bytes: VJ (ASCII letters)
// Third byte: flags
//   Values: least significant bit: 0: uncompressed, 1: compressed by deflate/gzip
// Fourth byte: version number: 1
//   Note: The version number should not be incremented unless the new format
//         cannot be used with older versions of the software.
//
// If the deflate/gzip flag is set, then the rest of the file must be decompressed
// before processing.
//
// The actual data is stored in chunks as below:
// +--------------------------------------------------------+
// | type (32 bits) | size (32 bits) | data ............... |
// +--------------------------------------------------------+
// type: A number corresponding to a specific subsystem (M68K/GPU/DSP/TOM/JERRY/...)
// size: the length of data in bytes
//
// The chunks may be stored in any order
//

//extern regstruct regs; // m68k regs

extern uint32_t pcQueue[0x400];
extern uint32_t a0Queue[0x400];
extern uint32_t a1Queue[0x400];
extern uint32_t a2Queue[0x400];
extern uint32_t a3Queue[0x400];
extern uint32_t a4Queue[0x400];
extern uint32_t a5Queue[0x400];
extern uint32_t a6Queue[0x400];
extern uint32_t a7Queue[0x400];
extern uint32_t d0Queue[0x400];
extern uint32_t d1Queue[0x400];
extern uint32_t d2Queue[0x400];
extern uint32_t d3Queue[0x400];
extern uint32_t d4Queue[0x400];
extern uint32_t d5Queue[0x400];
extern uint32_t d6Queue[0x400];
extern uint32_t d7Queue[0x400];
extern uint32_t pcQPtr;
extern bool startM68KTracing;


size_t jag_dump(FILE *fp)
{
	size_t total_dumped = 0;

	DUMP32(jaguarRunAddress);
	DUMPARR32(pcQueue);
	DUMPARR32(a0Queue);
	DUMPARR32(a1Queue);
	DUMPARR32(a2Queue);
	DUMPARR32(a3Queue);
	DUMPARR32(a4Queue);
	DUMPARR32(a5Queue);
	DUMPARR32(a6Queue);
	DUMPARR32(a7Queue);
	DUMPARR32(d0Queue);
	DUMPARR32(d1Queue);
	DUMPARR32(d2Queue);
	DUMPARR32(d3Queue);
	DUMPARR32(d4Queue);
	DUMPARR32(d5Queue);
	DUMPARR32(d6Queue);
	DUMPARR32(d7Queue);
	DUMP32(pcQPtr);
	DUMPBOOL(startM68KTracing);
	DUMPBOOL(bpmActive);
	DUMP32(bpmAddress1);

	DUMPARR32(regs.regs);
	DUMP32(regs.usp);
	DUMP32(regs.isp);
	DUMP16(regs.sr);
	DUMP8(regs.s);
	DUMP8(regs.stopped);
	DUMPINT(regs.intmask);
	DUMPINT(regs.intLevel);
	DUMPUINT(regs.c);
	DUMPUINT(regs.z);
	DUMPUINT(regs.n);
	DUMPUINT(regs.v);
	DUMPUINT(regs.x);
	DUMP32(regs.pc);
	DUMP32(regs.spcflags);
	DUMP32(regs.prefetch_pc);
	DUMP32(regs.prefetch);
	DUMPS32(regs.remainingCycles);
	DUMP32(regs.interruptCycles);
  
	uint32_t memsize = 0x200000;
	if (fwrite(jaguarMainRAM, 1, memsize, fp) != memsize)
	{
		WriteLog("DUMP RAM error\n");
		return -1;
	}
	total_dumped += memsize;

	return total_dumped;
}

size_t jag_load(FILE *fp)
{
	size_t total_loaded = 0;

	LOAD32(jaguarRunAddress);
	LOADARR32(pcQueue);
	LOADARR32(a0Queue);
	LOADARR32(a1Queue);
	LOADARR32(a2Queue);
	LOADARR32(a3Queue);
	LOADARR32(a4Queue);
	LOADARR32(a5Queue);
	LOADARR32(a6Queue);
	LOADARR32(a7Queue);
	LOADARR32(d0Queue);
	LOADARR32(d1Queue);
	LOADARR32(d2Queue);
	LOADARR32(d3Queue);
	LOADARR32(d4Queue);
	LOADARR32(d5Queue);
	LOADARR32(d6Queue);
	LOADARR32(d7Queue);
	LOAD32(pcQPtr);
	LOADBOOL(startM68KTracing);
	LOADBOOL(bpmActive);
	LOAD32(bpmAddress1);

	LOADARR32(regs.regs);
	LOAD32(regs.usp);
	LOAD32(regs.isp);
	LOAD16(regs.sr);
	LOAD8(regs.s);
	LOAD8(regs.stopped);
	LOADINT(regs.intmask);
	LOADINT(regs.intLevel);
	LOADUINT(regs.c);
	LOADUINT(regs.z);
	LOADUINT(regs.n);
	LOADUINT(regs.v);
	LOADUINT(regs.x);
	LOAD32(regs.pc);
	LOAD32(regs.spcflags);
	LOAD32(regs.prefetch_pc);
	LOAD32(regs.prefetch);
	LOADS32(regs.remainingCycles);
	LOAD32(regs.interruptCycles);

	uint32_t memsize = 0x200000;
	if (fread(jaguarMainRAM, 1, memsize, fp) != memsize)
	{
		WriteLog("LOAD RAM error\n");
		return -1;
	}
	total_loaded += memsize;

	return total_loaded;
}

size_t mem_dump(FILE *fp)
{
	size_t total_dumped = 0;

	// No need to dump the ROM
	uint32_t ramSize = 0x800000; //0xF20000;
	if (fwrite(jagMemSpace, 1, ramSize, fp) != ramSize)
	{
		WriteLog("DUMP MAIN RAM error\n");
		return -1;
	}
	total_dumped += ramSize;

	uint32_t otherSize = 0xF20000 - 0xDFFF00;
	if (fwrite(&jagMemSpace[0xDFFF00], 1, otherSize, fp) != otherSize)
	{
		WriteLog("DUMP OTHER RAM error\n");
		return -1;
	}
	total_dumped += otherSize;

	DUMP32(g_remain);
	DUMP16(asistat);
	DUMP32(d_remain);
	DUMP16(lrxd);
	DUMP16(rrxd);
	DUMP8(sstat);

	return total_dumped;
}

size_t mem_load(FILE *fp)
{
	size_t total_loaded = 0;

	uint32_t ramSize = 0x800000; //0xF20000;
	if (fread(jagMemSpace, 1, ramSize, fp) != ramSize)
	{
		WriteLog("LOAD MAIN RAM error\n");
		return -1;
	}
	total_loaded += ramSize;

	uint32_t otherSize = 0xF20000 - 0xDFFF00;
	if (fread(&jagMemSpace[0xDFFF00], 1, otherSize, fp) != otherSize)
	{
		WriteLog("LOAD OTHER RAM error\n");
		return -1;
	}
	total_loaded += otherSize;

	LOAD32(g_remain);
	LOAD16(asistat);
	LOAD32(d_remain);
	LOAD16(lrxd);
	LOAD16(rrxd);
	LOAD8(sstat);

	return total_loaded;
}

struct substate {
  uint32_t type;
  uint32_t size;
  size_t (*dump)(FILE *);
  size_t (*load)(FILE *);
};

typedef struct substate substate_t;

#define SUBSTATE(_type, _symbol) {_type, 0, _symbol##_dump, _symbol##_load}

static substate_t substates[] = {
	SUBSTATE(0x101, jag),
	SUBSTATE(0x102, m68k),
	SUBSTATE(0x103, mem),
	SUBSTATE(0x201, tom),
	SUBSTATE(0x301, jerry),
	SUBSTATE(0x401, gpu),
	SUBSTATE(0x501, dsp),
	SUBSTATE(0x601, blitter),
	SUBSTATE(0x602, op),
	SUBSTATE(0x603, events),
	SUBSTATE(0x604, dac),
	SUBSTATE(0x701, eeprom),
	//SUBSTATE(0x702, eeprom2),
	SUBSTATE(0x801, joystick),
	SUBSTATE(0x901, cdrom),
};

#define COMPATIBILITY_VERSION 0x01

// zlib deflate/inflate function prototypes
int def(FILE *source, FILE *dest, int level);
int inf(FILE *source, FILE *dest);

// [save state directory] / [ROMCRC32] - memdump - [slot number] .vjs
static const char *save_file_pattern = "%s%08X-memdump-%d.vjs";
static const char *save_file_pattern_tmp = "%s%08X-memdump-%d.vjs.tmp";
static char save_sprintf_buf[MAX_PATH+512];
int save_slot = 0;

size_t DumpSaveState(void)
{
	int r;

	if (save_slot == -1)
	{
		return -1;
	}
	else
	{
		bool compress = vjs.compressSaveStates;
		char const *file_pattern = compress ? save_file_pattern_tmp : save_file_pattern;
		sprintf(save_sprintf_buf, file_pattern, vjs.SaveStatePath, (unsigned int)jaguarMainROMCRC32, save_slot);
		FILE *fp = fopen(save_sprintf_buf, "wb");

		if (fp == NULL)
		{
			return -1;
		}
		else
		{
			uint32_t m68kPC = m68k_get_reg(NULL, M68K_REG_PC);
			WriteLog("SaveState file %s m68kPC: %08X\n", save_sprintf_buf, m68kPC);

			size_t total_dumped = 0;

			uint8_t magic[4] = { 'V', 'J', 0x00, COMPATIBILITY_VERSION };
			DUMP8(magic);

			for (int substate_idx = 0; substate_idx < sizeof(substates) / sizeof(substates[0]); substate_idx++)
			{
				substate_t *substate = &substates[substate_idx];
				DUMP32(substate->type);
				DUMP32(substate->size);
				size_t subtotal = substate->dump(fp);
				if (subtotal == -1)
				{
					WriteLog("SaveState file %s error dumping %04X\n", save_sprintf_buf, substate->type);
					goto error;
				}
				else
				{
					long size_offset = total_dumped - sizeof(substate->type);
					r = fseek(fp, size_offset, SEEK_SET);
					if (r == -1)
					{
						WriteLog("fseek error %d: %s\n", errno, strerror(errno));
						goto error;
					}
					else
					{
						substate->size = (uint32_t)(subtotal & 0x7fffffff);
						//WriteLog("dumping size %u at %ld\n", substate->size, size_offset);
						DUMP32(substate->size);
						total_dumped -= sizeof(substate->size);
						total_dumped += subtotal;
						r = fseek(fp, total_dumped, SEEK_SET);
						if (r == -1)
						{
							WriteLog("fseek error %d: %s\n", errno, strerror(errno));
							goto error;
						}
						else
						{
							WriteLog("SaveState file %s substate wrote %04X  size: %ld  pos now %ld\n", save_sprintf_buf, substate->type, subtotal, total_dumped);
						}
					}
				}
			}

			fflush(fp);
			fclose(fp);

			if (compress)
			{
				FILE *infp = fopen(save_sprintf_buf, "rb");
				if (fp == NULL)
				{
					WriteLog("SaveState file %s fopen failed\n", save_sprintf_buf);
					return -1;
				}
				else
				{
					sprintf(save_sprintf_buf, save_file_pattern, vjs.SaveStatePath, (unsigned int)jaguarMainROMCRC32, save_slot);
					WriteLog("compressing save state to %s\n", save_sprintf_buf);
					FILE *outfp = fopen(save_sprintf_buf, "wb");
					if (fp == NULL)
					{
						WriteLog("SaveState file %s fopen failed\n", save_sprintf_buf);
						fclose(infp);
						return -1;
					}
					else
					{
						magic[2] = 0x01;
						r = fwrite(magic, 1, 4, outfp);
						if (r != 4)
						{
							WriteLog("SaveState file %s fwrite failed. error %d: %s\n", save_sprintf_buf, errno, strerror(errno));
							fclose(outfp);
							fclose(infp);
							return -1;
						}
						else
						{
							r = fseek(infp, 4, SEEK_SET);
							if (r == -1)
							{
								WriteLog("fseek error %d: %s\n", errno, strerror(errno));
								fclose(outfp);
								fclose(infp);
								return -1;
							}
							else
							{
								// use the fastest compression level (1)
								r = def(infp, outfp, 1);
								if (r != Z_OK)
								{
									WriteLog("SaveState file %s deflate failed.", save_sprintf_buf);
									fclose(outfp);
									fclose(infp);
									return -1;
								}
								else
								{
									fflush(outfp);
									fclose(outfp);
									fclose(infp);
									sprintf(save_sprintf_buf, save_file_pattern_tmp, vjs.SaveStatePath, (unsigned int)jaguarMainROMCRC32, save_slot);
									remove(save_sprintf_buf);
								}
							}
						}
					}
				}
			}

			return total_dumped;

error:
			fclose(fp);
			return -1;
		}
	}
}

substate_t *find_substate(uint32_t type)
{
	for (int substate_idx = 0; substate_idx < sizeof(substates)/sizeof(substates[0]); substate_idx++)
	{
		substate_t *substate = &substates[substate_idx];
		if (type == substate->type)
		{
			return substate;
		}
	}
	return NULL;
}

size_t CanTryToLoadSaveState(void)
{
	if (save_slot == -1)
	{
		return -1;
	}
	else
	{
		sprintf(save_sprintf_buf, save_file_pattern, vjs.SaveStatePath, (unsigned int)jaguarMainROMCRC32, save_slot);
		FILE *fp = fopen(save_sprintf_buf, "rb");

		if (fp == NULL)
		{
			WriteLog("SaveState file %s fopen failed\n", save_sprintf_buf);
			return -1;
		}
		else
		{
			size_t total_loaded = 0;

			uint32_t magic;
			LOAD32(magic);
			fclose(fp);

			uint8_t firstByte = ((magic & 0xff000000) >> 24) & 0xff;
			uint8_t secondByte = ((magic & 0x00ff0000) >> 16) & 0xff;
			if ((firstByte != 'V') || (secondByte != 'J'))
			{
				WriteLog("SaveState file %s does not begin with VJ\n", save_sprintf_buf);
				return -1;
			}
			else
			{
				uint8_t compatibilityVersion = magic & 0xff;
				if (compatibilityVersion > COMPATIBILITY_VERSION)
				{
					WriteLog("SaveState file %s incompatible version 0x%02X > 0x%02X\n", save_sprintf_buf, compatibilityVersion, COMPATIBILITY_VERSION);
					return -1;
				}
				else
				{
					return 0;
				}
			}
		}
	}
}
 
size_t LoadSaveState(void)
{
	int r;

	if (save_slot == -1)
	{
		return -1;
	}
	else
	{
		sprintf(save_sprintf_buf, save_file_pattern, vjs.SaveStatePath, (unsigned int)jaguarMainROMCRC32, save_slot);
		FILE *fp = fopen(save_sprintf_buf, "rb");

		if (fp == NULL)
		{
			WriteLog("SaveState file %s fopen failed\n", save_sprintf_buf);
			return -1;
		}
		else
		{
			size_t total_loaded = 0;

			uint32_t magic;
			LOAD32(magic);
			uint8_t firstByte = ((magic & 0xff000000) >> 24) & 0xff;
			uint8_t secondByte = ((magic & 0x00ff0000) >> 16) & 0xff;
			if ((firstByte != 'V') || (secondByte != 'J'))
			{
				WriteLog("SaveState file %s does not begin with VJ\n", save_sprintf_buf);
				fclose(fp);
				return -1;
			}
			else
			{
				uint8_t compatibilityVersion = magic & 0xff;
				if (compatibilityVersion > COMPATIBILITY_VERSION)
				{
					WriteLog("SaveState file %s incompatible version 0x%02X > 0x%02X\n", save_sprintf_buf, compatibilityVersion, COMPATIBILITY_VERSION);
					fclose(fp);
					return -1;
				}
				else
				{
					uint8_t thirdByte = ((magic & 0x0000ff00) >> 8) & 0xff;
					bool compressed = (thirdByte & 0x01) == 0x01;
					if (compressed)
					{
						sprintf(save_sprintf_buf, save_file_pattern_tmp, vjs.SaveStatePath, (unsigned int)jaguarMainROMCRC32, save_slot);
						WriteLog("decompressing save state to %s\n", save_sprintf_buf);
						FILE *outfp = fopen(save_sprintf_buf, "wb");
						if (outfp == NULL)
						{
							WriteLog("SaveState file %s fopen failed\n", save_sprintf_buf);
							return -1;
						}
						else
						{
							uint8_t magicBytes[4] = { 'V', 'J', 0x00, compatibilityVersion };
							r = fwrite(magicBytes, 1, 4, outfp);
							if (r != 4)
							{
								WriteLog("SaveState file %s fwrite failed. error %d: %s\n", save_sprintf_buf, errno, strerror(errno));
								fclose(outfp);
								fclose(fp);
								return -1;
							}
							else
							{
								r = inf(fp, outfp);
								if (r != Z_OK)
								{
									WriteLog("SaveState file %s inflate failed.", save_sprintf_buf);
									fclose(outfp);
									fclose(fp);
									return -1;
								}
								else
								{
									fflush(outfp);
									fclose(outfp);
									fclose(fp);

									sprintf(save_sprintf_buf, save_file_pattern_tmp, vjs.SaveStatePath, (unsigned int)jaguarMainROMCRC32, save_slot);
									fp = fopen(save_sprintf_buf, "rb");
									if (fp == NULL)
									{
										WriteLog("SaveState file %s fopen failed after decompression\n", save_sprintf_buf);
										return -1;
									}
									else
									{
										total_loaded = 4;
										r = fseek(fp, total_loaded, SEEK_SET);
										if (r == -1)
										{
											WriteLog("SaveState fseek error %d: %s\n", errno, strerror(errno));
											goto error;
										}
									}
								}

								InitializeEventList();

								while (!feof(fp) && !ferror(fp))
								{
									//WriteLog("ftell: %ld  feof: %d\n", ftell(fp), feof(fp));
									uint32_t type;
									uint32_t size;

									if (fread(&type, sizeof(type), 1, fp) != 1)
									{
										break;
									}
									total_loaded += sizeof(type);
									type = FTOH32(type);

									LOAD32(size);
									substate_t *substate = find_substate(type);
									if (substate == NULL)
									{
										WriteLog("SaveState file %s substate %04X unknown. size: %u  at %ld\n", save_sprintf_buf, type, substate, size, total_loaded);
										total_loaded += size;
										r = fseek(fp, total_loaded, SEEK_SET);
										if (r == -1)
										{
											WriteLog("SaveState fseek error %d: %s\n", errno, strerror(errno));
											goto error;
										}
									}
									else
									{
										WriteLog("SaveState file %s substate %04X  size: %u  starting load at %ld\n", save_sprintf_buf, type, size, total_loaded);
										size_t subtotal = substate->load(fp);
										if (subtotal == -1)
										{
											WriteLog("SaveState file %s substate %04X load error\n", save_sprintf_buf, type);
											goto error;
										}
										WriteLog("SaveState file %s substate %04X  subtotal %ld  size: %u  at %ld\n", save_sprintf_buf, type, subtotal, size, total_loaded);
										total_loaded += size;
										size_t diff = size - subtotal;
										if (diff > 0)
										{
											// skip
											r = fseek(fp, total_loaded, SEEK_SET);
											if (r == -1)
											{
												WriteLog("SaveState fseek error %d: %s\n", errno, strerror(errno));
												goto error;
											}
										}
									}
								}
								//if (feof(fp)) {
								//  WriteLog("load eof at %ld  ftell: %ld\n", total_loaded, ftell(fp));
								//}
								if (ferror(fp))
								{
									WriteLog("SaveState load ferror %d: %s\n", errno, strerror(errno));
								}

								{
									uint32_t m68kPC = m68k_get_reg(NULL, M68K_REG_PC);
									WriteLog("SaveState file %s loaded m68kPC: %08X\n",	save_sprintf_buf, m68kPC);
								}

								fclose(fp);

								if (compressed)
								{
									sprintf(save_sprintf_buf, save_file_pattern_tmp, vjs.SaveStatePath, (unsigned int)jaguarMainROMCRC32, save_slot);
									remove(save_sprintf_buf);
								}

								return total_loaded;

error:
								fclose(fp);
								return -1;
							}
						}
					}
				}
			}
		}
	}
}

/* zpipe.c: example of proper use of zlib's inflate() and deflate()
   Not copyrighted -- provided to the public domain
   Version 1.4  11 December 2005  Mark Adler */

/* Version history:
   1.0  30 Oct 2004  First version
   1.1   8 Nov 2004  Add void casting for unused return values
                     Use switch statement for inflate() return values
   1.2   9 Nov 2004  Add assertions to document zlib guarantees
   1.3   6 Apr 2005  Remove incorrect assertion in inf()
   1.4  11 Dec 2005  Add hack to avoid MSDOS end-of-line conversions
                     Avoid some compiler warnings for input and output buffers
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
   def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_STREAM_ERROR if an invalid compression
   level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
   version of the library linked do not match, or Z_ERRNO if there is
   an error reading or writing the files. */
int def(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;

    /* compress until end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inf(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
#if 0
bool SaveState(void)
{
	return false;
}

bool LoadState(void)
{
	return false;
}
#endif
