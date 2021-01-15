//
// ELFManager.cpp: ELF format manager
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//  RG = Richard Goedeken
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM   Jan./2016  Created this file and added ELF format support
// JPM  07/13/2017  ELF DWARF format support improvement
// JPM  10/20/2018  Added function name support from ELF structure
// JPM  03/13/2020  Added ELF & DWARF .debug* types
//  RG   Jan./2021  Linux build fixes
//

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "libelf.h"
#include "gelf.h"
#include "libdwarf.h"
#include "log.h"
#include "ELFManager.h"
#include "DWARFManager.h"


//#define LOG_SUPPORT					// Support log


typedef struct {
	const char *SectionName;
	size_t SectionType;
}ELFSectionType;

typedef struct {
	size_t Type;
	size_t Size;
	union {
		Elf_Data *PtrDataTab;
		void *PtrTab;
	};
}ELFTab;


// Section type list
ELFSectionType	ELFTabSectionType[] =	{
	{ "", ELF_NULL_TYPE },
	{ ".text", ELF_text_TYPE },
	{ ".rodata", ELF_rodata_TYPE },
	{ ".data", ELF_data_TYPE },
	{ ".bss", ELF_bss_TYPE },
	{ ".heap", ELF_heap_TYPE },
	{ ".debug", ELF_debug_TYPE	},
	{ ".comment", ELF_comment_TYPE },
	{ ".shstrtab", ELF_shstrtab_TYPE },
	{ ".symtab", ELF_symtab_TYPE },
	{ ".strtab", ELF_strtab_TYPE },
	{ ".debug_abbrev", ELF_debug_abbrev_TYPE },
	{ ".debug_aranges", ELF_debug_aranges_TYPE },
	{ ".debug_frame", ELF_debug_frame_TYPE },
	{ ".debug_info", ELF_debug_info_TYPE },
	{ ".debug_line", ELF_debug_line_TYPE },
	{ ".debug_loc", ELF_debug_loc_TYPE },
	{ ".debug_macinfo", ELF_debug_macinfo_TYPE },
	{ ".debug_pubnames", ELF_debug_pubnames_TYPE },
	{ ".debug_pubtypes", ELF_debug_pubtypes_TYPE },
	{ ".debug_ranges", ELF_debug_ranges_TYPE },
	{ ".debug_str", ELF_debug_str_TYPE },
	{ ".debug_types", ELF_debug_types_TYPE }
};


// ELF management
Elf *ElfMem;
void *PtrExec;			// ELF executable

// ELF Dwarf management
bool	ElfDwarf;

// TAB structure
size_t	NbELFtabStruct;
ELFTab **ELFtab;


char *ELFManager_GetSymbolnameFromSymbolindex(size_t Index);


// ELF section type detection
size_t	ELFManager_GetSectionType(char *SectionName)
{
	size_t i;

	for (i = 0; i < ELF_END_TYPE; i++)
	{
		if (!strcmp(ELFTabSectionType[i].SectionName, SectionName))
		{
			return (ELFTabSectionType[i].SectionType);
		}
	}

	return (size_t)ELF_NO_TYPE;
}


// ELF manager executable copy
void	*ELFManager_ExeCopy(void *src, size_t size)
{
	if ((PtrExec = malloc(size)) != NULL)
	{
		memcpy(PtrExec, src, size);
	}

	return PtrExec;
}


Elf *ELFManager_MemOpen(char *PtrELFExe, size_t Size)
{
	return(ElfMem = elf_memory(PtrELFExe, Size));
}


int	ELFManager_MemEnd(void)
{
	int	err = 0;

	if (ElfMem)
	{
		err = elf_end(ElfMem);
		ElfMem = NULL;
	}

	return (err);
}


// ELF manager Initialisation
void	ELFManager_Init(void)
{
	PtrExec = NULL;
	NbELFtabStruct = 0;
	ELFtab = NULL;
	ElfMem = NULL;
	ElfDwarf = false;
}


// ELF manager Dwarf Initialisation
bool	ELFManager_DwarfInit(Elf *PtrElfMem, struct stat FileElfInfo)
{
	return (ElfDwarf = (DWARFManager_ElfInit(PtrElfMem, FileElfInfo) == DW_DLV_OK) ? true : false);
}


// ELF manager Reset
void	ELFManager_Reset(void)
{
	if (ElfDwarf)
	{
		DWARFManager_Reset();
		ElfDwarf = false;
	}

	if (PtrExec != NULL)
	{
		free(PtrExec);
		PtrExec = NULL;
	}

	if (ELFtab != NULL)
	{
		while (NbELFtabStruct)
		{
			free(ELFtab[--NbELFtabStruct]);
		}

		free(ELFtab);
		ELFtab = NULL;
	}

	ELFManager_MemEnd();
}


// ELF manager Closing
void	ELFManager_Close(void)
{
	ELFManager_Reset();
}


// Save the ELF tab in memory
bool ELFManager_AddTab(void *Ptr, size_t type)
{
	if (!NbELFtabStruct)
	{
		if ((ELFtab = (ELFTab **)calloc(1, sizeof(ELFTab *))) == NULL)
		{
			return false;
		}
	}
	else
	{
		if ((ELFtab = (ELFTab **)realloc(ELFtab, sizeof(ELFTab *)*(NbELFtabStruct+1))) == NULL)
		{
			return false;
		}
	}

	if ((ELFtab[NbELFtabStruct] = (ELFTab *)calloc(1, sizeof(ELFTab))) == NULL)
	{
		return false;
	}
	else
	{
		ELFtab[NbELFtabStruct]->Type = type;
		ELFtab[NbELFtabStruct]->PtrTab = Ptr;
		NbELFtabStruct++;
		return true;
	}
}


// Get Address from his Symbol Name
// Return 0 if Symbol name is not found
size_t ELFManager_GetAdrFromSymbolName(char *SymbolName)
{
	size_t Adr = 0;
	GElf_Sym *PtrST, ST;

	if (ELFtab && SymbolName)
	{
		for (size_t i = 0; i < NbELFtabStruct; i++)
		{
			if ((ELFtab[i]->Type == ELF_symtab_TYPE) && ((ELFtab[i]->PtrDataTab) != NULL))
			{
				int j = 0;

				while ((PtrST = gelf_getsym(ELFtab[i]->PtrDataTab, j++, &ST)) != NULL)
				{
					if (!strcmp(ELFManager_GetSymbolnameFromSymbolindex(PtrST->st_name), SymbolName))
					{
						Adr = PtrST->st_value;
					}
				}
			}
		}
	}

	return Adr;
}


// Get function name from his address
// Return NULL if function name is not found
char *ELFManager_GetFunctionName(size_t Adr)
{
	char *SymbolName = NULL;
	GElf_Sym *PtrST, ST;

	if (ELFtab != NULL)
	{
		for (size_t i = 0; i < NbELFtabStruct; i++)
		{
			if ((ELFtab[i]->Type == ELF_symtab_TYPE) && ((ELFtab[i]->PtrDataTab) != NULL))
			{
				int j = 0;

				while ((PtrST = gelf_getsym(ELFtab[i]->PtrDataTab, j++, &ST)) != NULL)
				{
					if (PtrST->st_value == Adr)
					{
						if (ELF32_ST_TYPE(PtrST->st_info) == STT_FUNC)
						{
							SymbolName = ELFManager_GetSymbolnameFromSymbolindex(PtrST->st_name);
						}
					}
				}
			}
		}
	}

	return SymbolName;
}


// Get Symbol name from his address
// Return NULL if Symbol name is not found
char *ELFManager_GetSymbolnameFromAdr(size_t Adr)
{
	char *SymbolName = NULL;
	GElf_Sym *PtrST, ST;

	if (ELFtab != NULL)
	{
		for (size_t i = 0; i < NbELFtabStruct; i++)
		{
			if ((ELFtab[i]->Type == ELF_symtab_TYPE) && ((ELFtab[i]->PtrDataTab) != NULL))
			{
				int j = 0;

				while ((PtrST = gelf_getsym(ELFtab[i]->PtrDataTab, j++, &ST)) != NULL)
				{
					if (PtrST->st_value == Adr)
					{
#ifdef LOG_SUPPORT
						WriteLog("ELF: .symtab: DATA: st_info=%0x, st_name=%0x, st_other=%0x, st_shndx=%0x, st_size=%0x, st_value=%0x\n", PtrST->st_info, PtrST->st_name, PtrST->st_other, PtrST->st_shndx, PtrST->st_size, PtrST->st_value);
#endif
						SymbolName = ELFManager_GetSymbolnameFromSymbolindex(PtrST->st_name);
					}
				}
			}
		}
	}

	return SymbolName;
}


// Get Symbol name from his Symbol index
char *ELFManager_GetSymbolnameFromSymbolindex(size_t Index)
{
	if (ELFtab != NULL)
	{
		for (size_t i = 0; i < NbELFtabStruct; i++)
		{
			if ((ELFtab[i]->Type == ELF_strtab_TYPE) && ((ELFtab[i]->PtrDataTab) != NULL))
			{
				return ((char *)(ELFtab[i]->PtrDataTab->d_buf) + Index);
			}
		}
	}

	return NULL;
}
