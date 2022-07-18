//
// Jaguar EEPROM handler
//
// by Cal2
// GCC/SDL port by Niels Wagenaar (Linux/WIN32) and Caz (BeOS)
// Cleanups/enhancements by James Hammons
// (C) 2010 Underground Software
//
// Patches
// https://atariage.com/forums/topic/243174-save-states-for-virtual-jaguar-patch/
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//  PL = PvtLewis <from Atari Age>
//
// Who  (MM/DD/YY)  What
// ---  ----------  ------------------------------------------------------------
// JLH  01/16/2010  Created this log ;-)
// JPM  10/11/2017  EEPROM directory detection and creation if missing
// JPM  11/18/2020  EEPROM directory creation allowed only for Windows
// JPM  March/2022  Added the save state patch from PvtLewis
//

#include "eeprom.h"
#if _WIN32 || _WIN64
#include <direct.h>
#else
#define _mkdir(dir) 1
#endif
#include <stdlib.h>
#include <string.h>								// For memset
#include "jaguar.h"
#include "log.h"
#include "settings.h"
#include "state.h"

#define eeprom_LOG

static uint16_t eeprom_ram[64];
static uint16_t cdromEEPROM[64];

//
// Private function prototypes
//

static void EEPROMSave(void);
static void eeprom_set_di(uint32_t state);
static void eeprom_set_cs(uint32_t state);
static uint32_t eeprom_get_do(void);
void ReadEEPROMFromFile(FILE * file, uint16_t * ram);
void WriteEEPROMToFile(FILE * file, uint16_t * ram);


enum { EE_STATE_START = 1, EE_STATE_OP_A, EE_STATE_OP_B, EE_STATE_0, EE_STATE_1,
	EE_STATE_2, EE_STATE_3, EE_STATE_0_0, EE_READ_ADDRESS, EE_STATE_0_0_0,
	EE_STATE_0_0_1, EE_STATE_0_0_2, EE_STATE_0_0_3, EE_STATE_0_0_1_0, EE_READ_DATA,
	EE_STATE_BUSY, EE_STATE_1_0, EE_STATE_1_1, EE_STATE_2_0, EE_STATE_3_0 };

// Local global variables

static uint16_t jerry_ee_state = EE_STATE_START;
static uint16_t jerry_ee_op = 0;
static uint16_t jerry_ee_rstate = 0;
static uint16_t jerry_ee_address_data = 0;
static uint16_t jerry_ee_address_cnt = 6;
static uint16_t jerry_ee_data = 0;
static uint16_t jerry_ee_data_cnt = 16;
static uint16_t jerry_writes_enabled = 0;
static uint16_t jerry_ee_direct_jump = 0;

static char eeprom_filename[MAX_PATH];
static char cdromEEPROMFilename[MAX_PATH];
static bool haveEEPROM = false;
static bool haveCDROMEEPROM = false;

size_t eeprom_dump(FILE *fp)
{
	size_t total_dumped = 0;

	DUMP16(jerry_ee_state);
	DUMP16(jerry_ee_op);
	DUMP16(jerry_ee_rstate);
	DUMP16(jerry_ee_address_data);
	DUMP16(jerry_ee_address_cnt);
	DUMP16(jerry_ee_data);
	DUMP16(jerry_ee_data_cnt);
	DUMP16(jerry_writes_enabled);
	DUMP16(jerry_ee_direct_jump);

	return total_dumped;
}

size_t eeprom_load(FILE *fp)
{
	size_t total_loaded = 0;

	LOAD16(jerry_ee_state);
	LOAD16(jerry_ee_op);
	LOAD16(jerry_ee_rstate);
	LOAD16(jerry_ee_address_data);
	LOAD16(jerry_ee_address_cnt);
	LOAD16(jerry_ee_data);
	LOAD16(jerry_ee_data_cnt);
	LOAD16(jerry_writes_enabled);
	LOAD16(jerry_ee_direct_jump);

	return total_loaded;
}

size_t eeprom2_dump(FILE *fp)
{
	size_t total_dumped = 0;

	DUMPARR16(eeprom_ram);
	DUMPARR16(cdromEEPROM);

	return total_dumped;
}

size_t eeprom2_load(FILE *fp)
{
	size_t total_loaded = 0;

	LOADARR16(eeprom_ram);
	LOADARR16(cdromEEPROM);

	return total_loaded;
}


// EEPROM initialisations
void EepromInit(void)
{
	FILE * fp;

	// No need for EEPROM for the Memory Track device :-P
	if (jaguarMainROMCRC32 == 0xFDF37F47)
	{
		WriteLog("EEPROM: Memory Track device detected...\n");
	}
	else
	{
		// Handle regular cartridge EEPROM
		sprintf(eeprom_filename, "%s%08X.eeprom", vjs.EEPROMPath, (unsigned int)jaguarMainROMCRC32);
		sprintf(cdromEEPROMFilename, "%scdrom.eeprom", vjs.EEPROMPath);
		fp = fopen(eeprom_filename, "rb");

		if (fp)
		{
			ReadEEPROMFromFile(fp, eeprom_ram);
			fclose(fp);
			WriteLog("EEPROM: Loaded from %s\n", eeprom_filename);
			haveEEPROM = true;
		}
		else
		{
			WriteLog("EEPROM: Could not open file \"%s\"!\n", eeprom_filename);
		}

		// Handle JagCD EEPROM
		sprintf(cdromEEPROMFilename, "%scdrom.eeprom", vjs.EEPROMPath);
		fp = fopen(cdromEEPROMFilename, "rb");

		if (fp)
		{
			ReadEEPROMFromFile(fp, cdromEEPROM);
			fclose(fp);
			WriteLog("EEPROM: Loaded from cdrom.eeprom\n");
			haveCDROMEEPROM = true;
		}
		else
		{
			WriteLog("EEPROM: Could not open file \"%s\"!\n", cdromEEPROMFilename);
		}
	}
}


// EEPROM reset
void EepromReset(void)
{
	if (!haveEEPROM)
	{
		memset(eeprom_ram, 0xFF, 64 * sizeof(uint16_t));
	}

	if (!haveCDROMEEPROM)
	{
		memset(cdromEEPROM, 0xFF, 64 * sizeof(uint16_t));
	}
}


//
void EepromDone(void)
{
	WriteLog("EEPROM: Done.\n");
}


// EEPROM save
static void EEPROMSave(void)
{
	FILE * fp;

	// Check if EEPROM directory exists and try to create it if not
	if (_mkdir(vjs.EEPROMPath))
	{
		WriteLog("EEPROM: Could not create directory \"%s!\"\n", vjs.EEPROMPath);
	}

	// Write out regular cartridge EEPROM data
	fp = fopen(eeprom_filename, "wb");
	if (fp)
	{
		WriteEEPROMToFile(fp, eeprom_ram);
		fclose(fp);
	}
	else
	{
		WriteLog("EEPROM: Could not create file \"%s!\"\n", eeprom_filename);
	}

	// Write out JagCD EEPROM data
	fp = fopen(cdromEEPROMFilename, "wb");
	if (fp)
	{
		WriteEEPROMToFile(fp, cdromEEPROM);
		fclose(fp);
	}
	else
	{
		WriteLog("EEPROM: Could not create file \"%s!\"\n", cdromEEPROMFilename);
	}
}


//
// Read/write EEPROM files to disk in an endian safe manner
//
void ReadEEPROMFromFile(FILE * file, uint16_t * ram)
{
	uint8_t buffer[128];
	size_t ignored = fread(buffer, 1, 128, file);

	for (int i = 0; i < 64; i++)
	{
		ram[i] = (buffer[(i * 2) + 0] << 8) | buffer[(i * 2) + 1];
	}
}


void WriteEEPROMToFile(FILE * file, uint16_t * ram)
{
	uint8_t buffer[128];

	for(int i=0; i<64; i++)
	{
		buffer[(i * 2) + 0] = ram[i] >> 8;
		buffer[(i * 2) + 1] = ram[i] & 0xFF;
	}

	fwrite(buffer, 1, 128, file);
}


uint8_t EepromReadByte(uint32_t offset)
{
	switch (offset)
	{
	case 0xF14001:
		return eeprom_get_do();
	case 0xF14801:
		break;
	case 0xF15001:
		eeprom_set_cs(1);
		break;
	default:
#ifdef eeprom_LOG
		WriteLog("EEPROM: unmapped 0x%.8x\n", offset);
#endif
		break;
	}

	return 0x00;
}


uint16_t EepromReadWord(uint32_t offset)
{
	return ((uint16_t)EepromReadByte(offset + 0) << 8) | EepromReadByte(offset + 1);
}


void EepromWriteByte(uint32_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xF14001:
		break;
	case 0xF14801:
		eeprom_set_di(data & 0x01);
		break;
	case 0xF15001:
		eeprom_set_cs(1);
		break;
	default:
#ifdef eeprom_LOG
		WriteLog("eeprom: unmapped 0x%.8x\n",offset);
#endif
		break;
	}
}


void EepromWriteWord(uint32_t offset, uint16_t data)
{
	EepromWriteByte(offset + 0, (data >> 8) & 0xFF);
	EepromWriteByte(offset + 1, data & 0xFF);
}


/*
;
;   Commands specific to the National Semiconductor NM93C14
;
;
;  9-bit commands..
;			 876543210
eEWDS	equ	%100000000		;Erase/Write disable (default)
eWRAL	equ	%100010000		;Writes all registers
eERAL	equ	%100100000		;Erase all registers
eEWEN	equ	%100110000		;Erase/write Enable
eWRITE	equ	%101000000		;Write selected register
eREAD	equ	%110000000		;read from EEPROM
eERASE	equ	%111000000		;Erase selected register
*/


static void eeprom_set_di(uint32_t data)
{
#ifdef eeprom_LOG
	WriteLog("eeprom: di=%i\n",data);
	WriteLog("eeprom: state %i\n",jerry_ee_state);
#endif

	switch (jerry_ee_state)
	{
	case EE_STATE_START:
		jerry_ee_state = EE_STATE_OP_A;
		break;
	case EE_STATE_OP_A:
		jerry_ee_op = (data << 1);
		jerry_ee_state = EE_STATE_OP_B;
		break;
	case EE_STATE_OP_B:
		jerry_ee_op |= data;
		jerry_ee_direct_jump = 0;
#ifdef eeprom_LOG
		WriteLog("eeprom: opcode %i\n",jerry_ee_op);
#endif

		switch (jerry_ee_op)
		{
		// Opcode 00: eEWEN, eERAL, eWRAL, eEWNDS
		case 0: jerry_ee_state = EE_STATE_0; break;
		// Opcode 01: eWRITE (Write selected register)
		case 1: jerry_ee_state = EE_STATE_1; break;
		// Opcode 10: eREAD (Read from EEPROM)
		case 2: jerry_ee_state = EE_STATE_2; break;
		// Opcode 11: eERASE (Erase selected register)
		case 3: jerry_ee_state = EE_STATE_3; break;
		}

		eeprom_set_di(data);
		break;
	case EE_STATE_0:
		jerry_ee_rstate = EE_STATE_0_0;
		jerry_ee_state = EE_READ_ADDRESS;
		jerry_ee_direct_jump = 1;
		jerry_ee_address_cnt = 6;
		jerry_ee_address_data = 0;
		break;
	case EE_STATE_0_0:
		switch ((jerry_ee_address_data >> 4) & 0x03)
		{
		// Opcode 00 00: eEWDS (Erase/Write disable)
		case 0: jerry_ee_state=EE_STATE_0_0_0; break;
		// Opcode 00 01: eWRAL (Write all registers)
		case 1: jerry_ee_state=EE_STATE_0_0_1; break;
		// Opcode 00 10: eERAL (Erase all registers)
		case 2: jerry_ee_state=EE_STATE_0_0_2; break;
		// Opcode 00 11: eEWEN (Erase/Write enable)
		case 3: jerry_ee_state=EE_STATE_0_0_3; break;
		}

		eeprom_set_di(data);
		break;
	case EE_STATE_0_0_0:
		// writes disable
#ifdef eeprom_LOG
		WriteLog("eeprom: read only\n");
#endif
		jerry_writes_enabled = 0;
		jerry_ee_state = EE_STATE_START;
		break;
	case EE_STATE_0_0_1:
		// writes all
		jerry_ee_rstate = EE_STATE_0_0_1_0;
		jerry_ee_state = EE_READ_DATA;
		jerry_ee_data_cnt = 16;
		jerry_ee_data = 0;
		jerry_ee_direct_jump = 1;
		break;
	case EE_STATE_0_0_1_0:
#ifdef eeprom_LOG
		WriteLog("eeprom: filling eeprom with 0x%.4x\n",data);
#endif
		if (jerry_writes_enabled)
		{
			for(int i=0; i<64; i++)
				eeprom_ram[i] = jerry_ee_data;

			EEPROMSave();						// Save it NOW!
		}

		//else
#ifdef eeprom_LOG
		WriteLog("eeprom: not writing because read only\n");
#endif
		jerry_ee_state = EE_STATE_BUSY;
		break;
	case EE_STATE_0_0_2:
		// erase all
#ifdef eeprom_LOG
		WriteLog("eeprom: erasing eeprom\n");
#endif
		if (jerry_writes_enabled)
			for(int i=0; i<64; i++)
				eeprom_ram[i] = 0xFFFF;

		jerry_ee_state = EE_STATE_BUSY;
		break;
	case EE_STATE_0_0_3:
		// writes enable
#ifdef eeprom_LOG
		WriteLog("eeprom: read/write\n");
#endif
		jerry_writes_enabled = 1;
		jerry_ee_state = EE_STATE_START;
		break;
	case EE_STATE_1:
		jerry_ee_rstate = EE_STATE_1_0;
		jerry_ee_state = EE_READ_ADDRESS;
		jerry_ee_address_cnt = 6;
		jerry_ee_address_data = 0;
		jerry_ee_direct_jump = 1;
		break;
	case EE_STATE_1_0:
		jerry_ee_rstate = EE_STATE_1_1;
		jerry_ee_state = EE_READ_DATA;
		jerry_ee_data_cnt = 16;
		jerry_ee_data = 0;
		jerry_ee_direct_jump = 1;
		break;
	case EE_STATE_1_1:
#ifdef eeprom_LOG
		WriteLog("eeprom: writing 0x%.4x at 0x%.2x\n",jerry_ee_data,jerry_ee_address_data);
#endif
		if (jerry_writes_enabled)
		{
			eeprom_ram[jerry_ee_address_data] = jerry_ee_data;
			EEPROMSave();						// Save it NOW!
		}

		jerry_ee_state = EE_STATE_BUSY;
		break;
	case EE_STATE_2:
		jerry_ee_rstate = EE_STATE_2_0;
		jerry_ee_state = EE_READ_ADDRESS;
		jerry_ee_address_cnt = 6;
		jerry_ee_address_data = 0;
		jerry_ee_data_cnt = 16;
		jerry_ee_data = 0;
		break;
	case EE_STATE_3:
		jerry_ee_rstate = EE_STATE_3_0;
		jerry_ee_state = EE_READ_ADDRESS;
		jerry_ee_address_cnt = 6;
		jerry_ee_address_data = 0;
		jerry_ee_direct_jump = 1;
		break;
	case EE_STATE_3_0:
#ifdef eeprom_LOG
		WriteLog("eeprom: erasing 0x%.2x\n",jerry_ee_address_data);
#endif
		if (jerry_writes_enabled)
			eeprom_ram[jerry_ee_address_data] = 0xFFFF;

		jerry_ee_state = EE_STATE_BUSY;
		break;
	case EE_READ_DATA:
#ifdef eeprom_LOG
		WriteLog("eeprom:\t\t\t%i bit %i\n",data,jerry_ee_data_cnt-1);
#endif
		jerry_ee_data <<= 1;
		jerry_ee_data |= data;
		jerry_ee_data_cnt--;

		if (!jerry_ee_data_cnt)
		{
			jerry_ee_state = jerry_ee_rstate;

			if (jerry_ee_direct_jump)
				eeprom_set_di(data);
		}

		break;
	case EE_READ_ADDRESS:
		jerry_ee_address_data <<= 1;
		jerry_ee_address_data |= data;
		jerry_ee_address_cnt--;
#ifdef eeprom_LOG
		WriteLog("eeprom:\t%i bits remaining\n",jerry_ee_address_cnt);
#endif

		if (!jerry_ee_address_cnt)
		{
			jerry_ee_state = jerry_ee_rstate;
#ifdef eeprom_LOG
			WriteLog("eeprom:\t\tread address 0x%.2x\n",jerry_ee_address_data);
#endif

			if (jerry_ee_direct_jump)
				eeprom_set_di(data);
		}

		break;
	default:
		jerry_ee_state = EE_STATE_OP_A;
	}
}


static void eeprom_set_cs(uint32_t state)
{
#ifdef eeprom_LOG
	WriteLog("eeprom: cs=%i\n",state);
#endif
	jerry_ee_state = EE_STATE_START;
	jerry_ee_op = 0;
	jerry_ee_rstate = 0;
	jerry_ee_address_data = 0;
	jerry_ee_address_cnt = 6;
	jerry_ee_data = 0;
	jerry_ee_data_cnt = 16;
	jerry_writes_enabled = 1;
}


static uint32_t eeprom_get_do(void)
{
	uint16_t data = 1;

	switch (jerry_ee_state)
	{
	case EE_STATE_START:
		data = 1;
		break;
	case EE_STATE_BUSY:
		jerry_ee_state = EE_STATE_START;
		data = 0;
		break;
	case EE_STATE_2_0:
		jerry_ee_data_cnt--;
		data = (eeprom_ram[jerry_ee_address_data] >> jerry_ee_data_cnt) & 0x01;

		if (!jerry_ee_data_cnt)
		{
#ifdef eeprom_LOG
			//WriteLog("eeprom: read 0x%.4x at 0x%.2x cpu %i pc=0x%.8x\n",eeprom_ram[jerry_ee_address_data],jerry_ee_address_data,jaguar_cpu_in_exec,s68000readPC());
#endif
			jerry_ee_state = EE_STATE_START;
		}
		break;
	}

#ifdef eeprom_LOG
	WriteLog("eeprom: do=%i\n",data);
#endif
	return data;
}

