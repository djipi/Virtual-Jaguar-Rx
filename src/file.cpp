//
// FILE.CPP
//
// File support
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//  RG = Richard Goedeken
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JLH  01/16/2010  Created this log ;-)
// JLH  02/28/2010  Added functions to look inside .ZIP files and handle contents
// JLH  06/01/2012  Added function to check ZIP file CRCs against file DB
// JPM        2016  Visual Studio support, ELF & DWARF format support, and Soft debugger support
// JPM  04/06/2019  Added ELF sections check
// JPM        2020  Added ELF section types check, new error messages and ELF executable file information
//  RG   Jan./2021  Linux build fixes
// JPM  06/23/2021  Added ELF sections check
//

#include "file.h"
#if defined(_MSC_VER)
#include "_MSC_VER/config.h"
#endif // _MSC_VER
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "crc32.h"
#include "filedb.h"
#include "eeprom.h"
#include "jaguar.h"
#include "log.h"
#include "memory.h"
#include "universalhdr.h"
#include "unzip.h"
#include "zlib.h"
#include "libelf.h"
#include "gelf.h"
#include "libdwarf.h"
#include "debugger/ELFManager.h"
#include "debugger/DBGManager.h"
#include "settings.h"


// Private function prototypes

static int gzfilelength(gzFile gd);
//#if defined(_MSC_VER) || defined(__MINGW64__)|| defined(__MINGW32__) || defined(__CYGWIN__)
static bool CheckExtension(const uint8_t *filename, const char *ext);
//#else
//static bool CheckExtension(const char * filename, const char * ext);
//#endif // _MSC_VER
//static int ParseFileType(uint8_t header1, uint8_t header2, uint32_t size);

// Private variables/enums


//
// Generic ROM loading
//
uint32_t JaguarLoadROM(uint8_t * &rom, char * path)
{
// We really should have some kind of sanity checking for the ROM size here to prevent
// a buffer overflow... !!! FIX !!!
#if defined(_MSC_VER)
#pragma message("Warning: !!! FIX !!! Should have sanity checking for ROM size to prevent buffer overflow!")
#else
#warning "!!! FIX !!! Should have sanity checking for ROM size to prevent buffer overflow!"
#endif // _MSC_VER
	uint32_t romSize = 0;

	WriteLog("FILE: JaguarLoadROM attempting to load file '%s'...", path);
	char * ext = strrchr(path, '.');

	// No filename extension == YUO FAIL IT (it is loading the file).
	// This is naive, but it works. But should probably come up with something a little
	// more robust, to prevent problems with dopes trying to exploit this.
	if (ext == NULL)
	{
		WriteLog("FAILED!\n");
		return 0;
	}

	WriteLog("\nFILE: Succeeded in finding extension (%s)!\n", ext);
	WriteLog("FILE: Loading \"%s\"...", path);

	if (strcasecmp(ext, ".zip") == 0)
	{
		// Handle ZIP file loading here...
		WriteLog("(ZIPped)...");

//		uint8_t * buffer = NULL;
//		romSize = GetFileFromZIP(path, FT_SOFTWARE, buffer);
		romSize = GetFileFromZIP(path, FT_SOFTWARE, rom);

		if (romSize == 0)
		{
			WriteLog("Failed!\n");
			return 0;
		}

//		memcpy(rom, buffer, romSize);
//		delete[] buffer;
	}
	else
	{
		// Handle gzipped files transparently [Adam Green]...

		gzFile fp = gzopen(path, "rb");

		if (fp == NULL)
		{
			WriteLog("Failed!\n");
			return 0;
		}

		romSize = gzfilelength(fp);
		rom = new uint8_t[romSize];
		gzseek(fp, 0, SEEK_SET);
		gzread(fp, rom, romSize);
		gzclose(fp);
	}

	WriteLog("OK (%i bytes)\n", romSize);

	return romSize;
}


//
// Jaguar file loading
// We do a more intelligent file analysis here instead of relying on (possible
// false) file extensions which people don't seem to give two shits about
// anyway. :-(
//
bool JaguarLoadFile(char * path)
{
	Elf *ElfMem;
	GElf_Ehdr ElfEhdr, *PtrGElfEhdr;
	Elf_Scn	*PtrElfScn;
	Elf_Data	*PtrElfData;
	GElf_Shdr GElfShdr, *PtrGElfShdr;
	size_t NbrSect;
	uint8_t *buffer = NULL;
	char *NameSection;
	size_t ElfSectionNameType;
	int	DBGType = DBG_NO_TYPE;
	bool error;
	int err;
	struct stat _statbuf;

	jaguarROMSize = JaguarLoadROM(buffer, path);

	if (jaguarROMSize == 0)
	{
		// It's up to the GUI to report errors, not us. :-)
		WriteLog("FILE: Could not load ROM from file \"%s\"...\nAborting load!\n", path);
		return false;
	}

	jaguarMainROMCRC32 = crc32_calcCheckSum(buffer, jaguarROMSize);
	WriteLog("CRC: %08X\n", (unsigned int)jaguarMainROMCRC32);
// TODO: Check for EEPROM file in ZIP file. If there is no EEPROM in the user's EEPROM
//       directory, copy the one from the ZIP file, if it exists.
	EepromInit();
	jaguarRunAddress = 0x802000;					// For non-BIOS runs, this is true
	int fileType = ParseFileType(buffer, jaguarROMSize);
	jaguarCartInserted = false;
	DBGManager_Reset();

	if (fileType == JST_ROM)
	{
		jaguarCartInserted = true;
		memcpy(jagMemSpace + 0x800000, buffer, jaguarROMSize);
// Checking something...
jaguarRunAddress = GET32(jagMemSpace, 0x800404);
WriteLog("FILE: Cartridge run address is reported as $%X...\n", jaguarRunAddress);
		delete[] buffer;
		return true;
	}
	else if (fileType == JST_ALPINE)
	{
		// File extension ".ROM": Alpine image that loads/runs at $802000
		WriteLog("FILE: Setting up Alpine ROM... Run address: 00802000, length: %08X\n", jaguarROMSize);
		memset(jagMemSpace + 0x800000, 0xFF, 0x2000);
		memcpy(jagMemSpace + 0x802000, buffer, jaguarROMSize);
		delete[] buffer;

// Maybe instead of this, we could try requiring the STUBULATOR ROM? Just a thought...
		// Try setting the vector to say, $1000 and putting an instruction there that loops forever:
		// This kludge works! Yeah!
		SET32(jaguarMainRAM, 0x10, 0x00001000);
		SET16(jaguarMainRAM, 0x1000, 0x60FE);		// Here: bra Here
		return true;
	}
	else if (fileType == JST_ELF32)
	{
		DBGType = DBG_ELF;

		char *PtrELFExe = (char *)ELFManager_ExeCopy(buffer, jaguarROMSize);

		if (PtrELFExe != NULL)
		{
			// check the ELF version
			if ((elf_version(EV_CURRENT) != EV_NONE) && (ElfMem = ELFManager_MemOpen(PtrELFExe, jaguarROMSize)))
			{
				// get the file information
				stat(path, &_statbuf);

				if (ELFManager_DwarfInit(ElfMem, _statbuf))
				{
					DBGType |= DBG_ELFDWARF;
				}

				if (!elf_getshdrnum(ElfMem, &NbrSect))
				{
					if (((PtrGElfEhdr = gelf_getehdr(ElfMem, &ElfEhdr)) != NULL) && ((PtrElfScn = elf_getscn(ElfMem, 0)) != NULL))
					{
						for (error = false; (PtrElfScn != NULL) && (error == false); PtrElfScn = elf_nextscn(ElfMem, PtrElfScn))
						{
							PtrElfData = NULL;

							if ((PtrGElfShdr = gelf_getshdr(PtrElfScn, &GElfShdr)) == NULL)
							{
								error = true;
							}
							else
							{
								NameSection = elf_strptr(ElfMem, PtrGElfEhdr->e_shstrndx, (size_t)PtrGElfShdr->sh_name);
								WriteLog("FILE: ELF Section %s found\n", NameSection);

								if (((ElfSectionNameType = ELFManager_GetSectionType(NameSection)) == ELF_NO_TYPE) && vjs.ELFSectionsCheck)
								{
									WriteLog("FILE: ELF Section %s not recognized\n", NameSection);
									error = true;
								}
								else
								{
									switch (PtrGElfShdr->sh_type)
									{
									case SHT_NULL:
										break;

									case SHT_PROGBITS:
										if ((PtrGElfShdr->sh_flags & (SHF_ALLOC | SHF_WRITE | SHF_EXECINSTR)))
										{
											if (PtrGElfShdr->sh_addr >= 0x800000)
											{
												memcpy(jagMemSpace + PtrGElfShdr->sh_addr, buffer + PtrGElfShdr->sh_offset, PtrGElfShdr->sh_size);
												//error = false;
											}
											else
											{
												memcpy(jaguarMainRAM + PtrGElfShdr->sh_addr, buffer + PtrGElfShdr->sh_offset, PtrGElfShdr->sh_size);
											}
										}
										else
										{
											switch (ElfSectionNameType)
											{
											case ELF_debug_TYPE:
											case ELF_debug_abbrev_TYPE:
											case ELF_debug_aranges_TYPE:
											case ELF_debug_frame_TYPE:
											case ELF_debug_info_TYPE:
											case ELF_debug_line_TYPE:
											case ELF_debug_loc_TYPE:
											case ELF_debug_macinfo_TYPE:
											case ELF_debug_pubnames_TYPE:
											case ELF_debug_pubtypes_TYPE:
											case ELF_debug_ranges_TYPE:
											case ELF_debug_str_TYPE:
											case ELF_debug_types_TYPE:						
												break;

											case ELF_stab_TYPE:
												break;

											case ELF_heap_TYPE:
												break;

											case ELF_comment_TYPE:
												break;

											default:
												WriteLog("FILE: ELF section %s is not recognized\n", NameSection);
												error = true;
												break;
											}
										}
										break;

									case SHT_NOBITS:
										break;

									case SHT_STRTAB:
									case SHT_SYMTAB:
										while ((error == false) && ((PtrElfData = elf_getdata(PtrElfScn, PtrElfData)) != NULL))
										{
											if (!ELFManager_AddTab(PtrElfData, ElfSectionNameType))
											{
												WriteLog("FILE: ELF tab cannot be allocated\n");
												error = true;
											}
										}
										break;

									default:
										WriteLog("FILE: ELF SHT type %i not recognized\n", PtrGElfShdr->sh_type);
										error = true;
										break;
									}
								}
							}
						}

						// Set the executable address
						jaguarRunAddress = (uint32_t)PtrGElfEhdr->e_entry;
						WriteLog("FILE: Setting up ELF 32bits... Run address: %08X\n", jaguarRunAddress);
					}
					else
					{
						error = true;
					}
				}
				else
				{
					WriteLog("FILE: Cannot get the number of the ELF sections\n");
					error = true;
				}
			}
			else
			{
				error = true;
				WriteLog("FILE: libelf version is not recognized or libelf memory cannot be opened\n");
			}
		}
		else
		{
			error = true;
			WriteLog("FILE: ELFManager cannot allocate memory\n");
		}

		delete[] buffer;

		if (error)
		{
			WriteLog("FILE: ELF parsing error\n");

			if ((err = elf_errno()))
			{
				WriteLog("FILE: ELF error: %s\n", elf_errmsg(err));
			}

			return false;
		}
		else
		{
			DBGManager_SetType(DBGType);
			return true;
		}
	}
	else if (fileType == JST_ABS_TYPE1)
	{
		// For ABS type 1, run address == load address
		uint32_t loadAddress = GET32(buffer, 0x16),
			codeSize = GET32(buffer, 0x02) + GET32(buffer, 0x06);
		WriteLog("FILE: Setting up homebrew (ABS-1)... Run address: %08X, length: %08X\n", loadAddress, codeSize);
		memcpy(jagMemSpace + loadAddress, buffer + 0x24, codeSize);
		delete[] buffer;
		jaguarRunAddress = loadAddress;
		return true;
	}
	else if (fileType == JST_ABS_TYPE2)
	{
		uint32_t loadAddress = GET32(buffer, 0x28), runAddress = GET32(buffer, 0x24),
			codeSize = GET32(buffer, 0x18) + GET32(buffer, 0x1C);
		WriteLog("FILE: Setting up homebrew (ABS-2)... Run address: %08X, length: %08X\n", runAddress, codeSize);
		memcpy(jagMemSpace + loadAddress, buffer + 0xA8, codeSize);
		delete[] buffer;
		jaguarRunAddress = runAddress;
		return true;
	}
	// NB: This is *wrong*
	/*
	Basically, if there is no "JAG" at position $1C, then the long there is the load/start
	address in LITTLE ENDIAN.
	If "JAG" is present, the the next character ("R" or "L") determines the size of the
	JagServer command (2 bytes vs. 4). Following that are the commands themselves;
	typically it will either be 2 (load) or 3 (load & run). Command headers go like so:
	2:
	Load address (long)
	Length (long)
	payload
	3:
	Load address (long)
	Length (long)
	Run address (long)
	payload
	5: (Reset)
	[command only]
	7: (Run at address)
	Run address (long)
	[no payload]
	9: (Clear memory)
	Start address (long)
	End address (long)
	[no payload]
	10: (Poll for commands)
	[command only]
	12: (Load & run user program)
	filname, terminated with NULL
	[no payload]
	$FFFF: (Halt)
	[no payload]
	*/
	else if (fileType == JST_JAGSERVER)
	{
		// This kind of shiaut should be in the detection code below...
		// (and now it is! :-)
//		if (buffer[0x1C] == 'J' && buffer[0x1D] == 'A' && buffer[0x1E] == 'G')
//		{
			// Still need to do some checking here for type 2 vs. type 3. This assumes 3
			// Also, JAGR vs. JAGL (word command size vs. long command size)
			uint32_t loadAddress = GET32(buffer, 0x22), runAddress = GET32(buffer, 0x2A);
			WriteLog("FILE: Setting up homebrew (Jag Server)... Run address: $%X, length: $%X\n", runAddress, jaguarROMSize - 0x2E);
			memcpy(jagMemSpace + loadAddress, buffer + 0x2E, jaguarROMSize - 0x2E);
			delete[] buffer;
			jaguarRunAddress = runAddress;

// Hmm. Is this kludge necessary?
SET32(jaguarMainRAM, 0x10, 0x00001000);		// Set Exception #4 (Illegal Instruction)
SET16(jaguarMainRAM, 0x1000, 0x60FE);		// Here: bra Here

			return true;
//		}
//		else // Special WTFOMGBBQ type here...
//		{
//			uint32_t loadAddress = (buffer[0x1F] << 24) | (buffer[0x1E] << 16) | (buffer[0x1D] << 8) | buffer[0x1C];
//			WriteLog("FILE: Setting up homebrew (GEMDOS WTFOMGBBQ type)... Run address: $%X, length: $%X\n", loadAddress, jaguarROMSize - 0x20);
//			memcpy(jagMemSpace + loadAddress, buffer + 0x20, jaguarROMSize - 0x20);
//			delete[] buffer;
//			jaguarRunAddress = loadAddress;
//			return true;
//		}
	}
	else if (fileType == JST_WTFOMGBBQ)
	{
		uint32_t loadAddress = (buffer[0x1F] << 24) | (buffer[0x1E] << 16) | (buffer[0x1D] << 8) | buffer[0x1C];
		WriteLog("FILE: Setting up homebrew (GEMDOS WTFOMGBBQ type)... Run address: $%X, length: $%X\n", loadAddress, jaguarROMSize - 0x20);
		memcpy(jagMemSpace + loadAddress, buffer + 0x20, jaguarROMSize - 0x20);
		delete[] buffer;
		jaguarRunAddress = loadAddress;
		return true;
	}

	// We can assume we have JST_NONE at this point. :-P
	WriteLog("FILE: Failed to load headerless file.\n");
	return false;
}


//
// "Debugger" file loading
// To keep the things separate between "Debugger" and "Alpine" loading until usage clarification has been done
//
bool DebuggerLoadFile(char * path)
{
	return (AlpineLoadFile(path));
}


//
// "Alpine" file loading
// Since the developers were coming after us with torches and pitchforks, we
// decided to allow this kind of thing. ;-) But ONLY FOR THE DEVS, DAMMIT! >:-U
// O_O
//
bool AlpineLoadFile(char * path)
{
	uint8_t * buffer = NULL;
	jaguarROMSize = JaguarLoadROM(buffer, path);

	if (jaguarROMSize == 0)
	{
		// It's up to the GUI to deal with failure, not us. ;-)
		WriteLog("FILE: Could not load Alpine from file \"%s\"...\nAborting load!\n", path);
		return false;
	}

	jaguarMainROMCRC32 = crc32_calcCheckSum(buffer, jaguarROMSize);
	WriteLog("FILE: CRC is %08X\n", (unsigned int)jaguarMainROMCRC32);
	EepromInit();

	jaguarRunAddress = 0x802000;

	WriteLog("FILE: Setting up Alpine ROM with non-standard length... Run address: 00802000, length: %08X\n", jaguarROMSize);

	memset(jagMemSpace + 0x800000, 0xFF, 0x2000);
	memcpy(jagMemSpace + 0x802000, buffer, jaguarROMSize);
	delete[] buffer;

// Maybe instead of this, we could try requiring the STUBULATOR ROM? Just a thought...
	// Try setting the vector to say, $1000 and putting an instruction there
	// that loops forever:
	// This kludge works! Yeah!
	SET32(jaguarMainRAM, 0x10, 0x00001000);		// Set Exception #4 (Illegal Instruction)
	SET16(jaguarMainRAM, 0x1000, 0x60FE);		// Here: bra Here

	return true;
}


//
// Get the length of a (possibly) gzipped file
//
static int gzfilelength(gzFile gd)
{
   int size = 0, length = 0;
   unsigned char buffer[0x10000];

   gzrewind(gd);

   do
   {
      // Read in chunks until EOF
      size = gzread(gd, buffer, 0x10000);

      if (size <= 0)
      	break;

      length += size;
   }
   while (!gzeof(gd));

   gzrewind(gd);
   return length;
}


//
// Compare extension to passed in filename. If equal, return true; otherwise false.
//
//#if defined(_MSC_VER) || defined(__MINGW64__)|| defined(__MINGW32__) || defined(__CYGWIN__)
static bool CheckExtension(const uint8_t *filename, const char *ext)
//#else
//static bool CheckExtension(const char * filename, const char * ext)
//#endif // _MSC_VER
{
	// Sanity checking...
	if ((filename == NULL) || (ext == NULL))
		return false;

	const char * filenameExt = strrchr((const char *)filename, '.');	// Get the file's extension (if any)

	if (filenameExt == NULL)
		return false;

	return (strcasecmp(filenameExt, ext) == 0 ? true : false);
}


//
// Get file from .ZIP
// Returns the size of the file inside the .ZIP file that we're looking at
// NOTE: If the thing we're looking for is found, it allocates it in the passed in buffer.
//       Which means we have to deallocate it later.
//
uint32_t GetFileFromZIP(const char * zipFile, FileType type, uint8_t * &buffer)
{
// NOTE: We could easily check for this by discarding anything that's larger than the RAM/ROM
//       size of the Jaguar console.
#if defined(_MSC_VER)
#pragma message("Warning: !!! FIX !!! Should have sanity checking for ROM size to prevent buffer overflow!")
#else
#warning "!!! FIX !!! Should have sanity checking for ROM size to prevent buffer overflow!"
#endif // _MSC_VER
	const char ftStrings[5][32] = { "Software", "EEPROM", "Label", "Box Art", "Controller Overlay" };
//	ZIP * zip = openzip(0, 0, zipFile);
	FILE * zip = fopen(zipFile, "rb");

	if (zip == NULL)
	{
		WriteLog("FILE: Could not open file '%s'!\n", zipFile);
		return 0;
	}

//	zipent * ze;
	ZipFileEntry ze;
	bool found = false;

	// The order is here is important: If the file is found, we need to short-circuit the
	// readzip() call because otherwise, 'ze' will be pointing to the wrong file!
//	while (!found && readzip(zip))
	while (!found && GetZIPHeader(zip, ze))
	{
//		ze = &zip->ent;

		// Here we simply rely on the file extension to tell the truth, but we know
		// that extensions lie like sons-a-bitches. So this is naive, we need to do
		// something a little more robust to keep bad things from happening here.
#if defined(_MSC_VER)
#pragma message("Warning: !!! Checking for image by extension can be fooled !!!")
#else
#warning "!!! Checking for image by extension can be fooled !!!"
#endif // _MSC_VER
		if ((type == FT_LABEL) && (CheckExtension(ze.filename, ".png") || CheckExtension(ze.filename, ".jpg") || CheckExtension(ze.filename, ".gif")))
		{
			found = true;
			WriteLog("FILE: Found image file '%s'.\n", ze.filename);
		}

		if ((type == FT_SOFTWARE) && (CheckExtension(ze.filename, ".j64")
			|| CheckExtension(ze.filename, ".rom") || CheckExtension(ze.filename, ".abs")
			|| CheckExtension(ze.filename, ".cof") || CheckExtension(ze.filename, ".coff")
			|| CheckExtension(ze.filename, ".jag") || CheckExtension(ze.filename, ".elf")))
		{
			found = true;
			WriteLog("FILE: Found software file '%s'.\n", ze.filename);
		}

		if ((type == FT_EEPROM) && (CheckExtension(ze.filename, ".eep") || CheckExtension(ze.filename, ".eeprom")))
		{
			found = true;
			WriteLog("FILE: Found EEPROM file '%s'.\n", ze.filename);
		}

		if (!found)
			fseek(zip, ze.compressedSize, SEEK_CUR);
	}

	uint32_t fileSize = 0;

	if (found)
	{
		WriteLog("FILE: Uncompressing...");
// Insert file size sanity check here...
		buffer = new uint8_t[ze.uncompressedSize];

//		if (readuncompresszip(zip, ze.compressedSize, buffer) == 0)
//		if (UncompressFileFromZIP(zip, ze.compressedSize, buffer) == 0)
		if (UncompressFileFromZIP(zip, ze, buffer) == 0)
		{
			fileSize = ze.uncompressedSize;
			WriteLog("success! (%u bytes)\n", fileSize);
		}
		else
		{
			delete[] buffer;
			buffer = NULL;
			WriteLog("FAILED!\n");
		}
	}
	else
		// Didn't find what we're looking for...
		WriteLog("FILE: Failed to find file of type %s...\n", ftStrings[type]);

//	closezip(zip);
	fclose(zip);
	return fileSize;
}


uint32_t GetFileDBIdentityFromZIP(const char * zipFile)
{
	FILE * zip = fopen(zipFile, "rb");

	if (zip == NULL)
	{
		WriteLog("FILE: Could not open file '%s'!\n", zipFile);
		return 0;
	}

	ZipFileEntry ze;

	// Loop through all files in the zip file under consideration
	while (GetZIPHeader(zip, ze))
	{
		// & loop through all known CRC32s in our file DB to see if it's there!
		uint32_t index = 0;

		while (romList[index].crc32 != 0xFFFFFF)
		{
			if (romList[index].crc32 == ze.crc32)
			{
				fclose(zip);
				return index;
			}

			index++;
		}

		// We didn't find it, so skip the compressed data...
		fseek(zip, ze.compressedSize, SEEK_CUR);
	}

	fclose(zip);
	return (uint32_t )-1;
}


bool FindFileInZIPWithCRC32(const char * zipFile, uint32_t crc)
{
	FILE * zip = fopen(zipFile, "rb");

	if (zip == NULL)
	{
		WriteLog("FILE: Could not open file '%s'!\n", zipFile);
		return 0;
	}

	ZipFileEntry ze;

	// Loop through all files in the zip file under consideration
	while (GetZIPHeader(zip, ze))
	{
		if (ze.crc32 == crc)
		{
			fclose(zip);
			return true;
		}

		fseek(zip, ze.compressedSize, SEEK_CUR);
	}

	fclose(zip);
	return false;
}


//
// Parse the file type based upon file size and/or headers.
//
uint32_t ParseFileType(uint8_t * buffer, uint32_t size)
{
	// Check headers first...

	// ELF 32bits
	if (buffer[EI_CLASS] == ELFCLASS32)
	{
		if (((BigToLittleEndian16(((Elf32_Ehdr *)buffer)->e_machine) & 0xFF) == EM_68K) && (BigToLittleEndian16(((Elf32_Ehdr *)buffer)->e_type) == ET_EXEC) && (buffer[0] == ELFMAG0) && (buffer[1] == ELFMAG1) && (buffer[2] == ELFMAG2) && (buffer[3] == ELFMAG3))
			return JST_ELF32;
	}

	// ABS/COFF type 1
	if (buffer[0] == 0x60 && buffer[1] == 0x1B)
		return JST_ABS_TYPE1;

	// ABS/COFF type 2
	if (buffer[0] == 0x01 && buffer[1] == 0x50)
		return JST_ABS_TYPE2;

	// Jag Server & other old shite
	if (buffer[0] == 0x60 && buffer[1] == 0x1A)
	{
		if (buffer[0x1C] == 'J' && buffer[0x1D] == 'A' && buffer[0x1E] == 'G')
			return JST_JAGSERVER;
		else
			return JST_WTFOMGBBQ;
	}

	// And if that fails, try file sizes...

	// If the file size is divisible by 1M, we probably have an regular ROM.
	// We can also check our CRC32 against the internal ROM database to be sure.
	// (We also check for the Memory Track cartridge size here as well...)
	if ((size % 1048576) == 0 || size == 131072)
		return JST_ROM;

	// If the file size + 8192 bytes is divisible by 1M, we probably have an
	// Alpine format ROM.
	if (((size + 8192) % 1048576) == 0)
		return JST_ALPINE;

	// Headerless crap
	return JST_NONE;
}

//
// Check for universal header
//
bool HasUniversalHeader(uint8_t * rom, uint32_t romSize)
{
	// Sanity check
	if (romSize < 8192)
		return false;

	for(int i=0; i<8192; i++)
		if (rom[i] != universalCartHeader[i])
			return false;

	return true;
}

#if 0
// Misc. doco

/*
Stubulator ROM vectors...
handler 001 at $00E00008
handler 002 at $00E008DE
handler 003 at $00E008E2
handler 004 at $00E008E6
handler 005 at $00E008EA
handler 006 at $00E008EE
handler 007 at $00E008F2
handler 008 at $00E0054A
handler 009 at $00E008FA
handler 010 at $00000000
handler 011 at $00000000
handler 012 at $00E008FE
handler 013 at $00E00902
handler 014 at $00E00906
handler 015 at $00E0090A
handler 016 at $00E0090E
handler 017 at $00E00912
handler 018 at $00E00916
handler 019 at $00E0091A
handler 020 at $00E0091E
handler 021 at $00E00922
handler 022 at $00E00926
handler 023 at $00E0092A
handler 024 at $00E0092E
handler 025 at $00E0107A
handler 026 at $00E0107A
handler 027 at $00E0107A
handler 028 at $00E008DA
handler 029 at $00E0107A
handler 030 at $00E0107A
handler 031 at $00E0107A
handler 032 at $00000000

Let's try setting up the illegal instruction vector for a stubulated jaguar...

		SET32(jaguar_mainRam, 0x08, 0x00E008DE);
		SET32(jaguar_mainRam, 0x0C, 0x00E008E2);
		SET32(jaguar_mainRam, 0x10, 0x00E008E6);	// <-- Should be here (it is)...
		SET32(jaguar_mainRam, 0x14, 0x00E008EA);//*/

/*
ABS Format sleuthing (LBUGDEMO.ABS):

000000  60 1B 00 00 05 0C 00 04 62 C0 00 00 04 28 00 00
000010  12 A6 00 00 00 00 00 80 20 00 FF FF 00 80 25 0C
000020  00 00 40 00

DRI-format file detected...
Text segment size = 0x0000050c bytes
Data segment size = 0x000462c0 bytes
BSS Segment size = 0x00000428 bytes
Symbol Table size = 0x000012a6 bytes
Absolute Address for text segment = 0x00802000
Absolute Address for data segment = 0x0080250c
Absolute Address for BSS segment = 0x00004000

(CRZDEMO.ABS):
000000  01 50 00 03 00 00 00 00 00 03 83 10 00 00 05 3b
000010  00 1c 00 03 00 00 01 07 00 00 1d d0 00 03 64 98
000020  00 06 8b 80 00 80 20 00 00 80 20 00 00 80 3d d0

000030  2e 74 78 74 00 00 00 00 00 80 20 00 00 80 20 00 .txt (+36 bytes)
000040  00 00 1d d0 00 00 00 a8 00 00 00 00 00 00 00 00
000050  00 00 00 00 00 00 00 20
000058  2e 64 74 61 00 00 00 00 00 80 3d d0 00 80 3d d0 .dta (+36 bytes)
000068  00 03 64 98 00 00 1e 78 00 00 00 00 00 00 00 00
000078  00 00 00 00 00 00 00 40
000080  2e 62 73 73 00 00 00 00 00 00 50 00 00 00 50 00 .bss (+36 bytes)
000090  00 06 8b 80 00 03 83 10 00 00 00 00 00 00 00 00
0000a0  00 00 00 00 00 00 00 80

Header size is $A8 bytes...

BSD/COFF format file detected...
3 sections specified
Symbol Table offset = 230160				($00038310)
Symbol Table contains 1339 symbol entries	($0000053B)
The additional header size is 28 bytes		($001C)
Magic Number for RUN_HDR = 0x00000107
Text Segment Size = 7632					($00001DD0)
Data Segment Size = 222360					($00036498)
BSS Segment Size = 428928					($00068B80)
Starting Address for executable = 0x00802000
Start of Text Segment = 0x00802000
Start of Data Segment = 0x00803dd0
*/
#endif
