

#ifndef __ELFMANAGER_H__
#define __ELFMANAGER_H__


typedef enum {
	ELF_NO_TYPE = -1,
	ELF_NULL_TYPE = 0,
	ELF_text_TYPE,					// Executable instructions, of a program
	ELF_rodata_TYPE,				// Read-only data that typically contribute to a non-writable segment in the process image
	ELF_data_TYPE,					// Initialized data that contribute to the program's memory image
	ELF_bss_TYPE,					// Data that contributes to the program's memory image as uninitialized
	ELF_heap_TYPE,
	ELF_debug_TYPE,					// Information for symbolic debugging (the contents are unspecified)
	ELF_comment_TYPE,				// Version control information
	ELF_stab_TYPE,					// Debugging information
	ELF_stabstr_TYPE,				// Strings associated with the debugging infomation contained in the .stab section
	ELF_shstrtab_TYPE,				// Section names
	ELF_symtab_TYPE,				// Symbol table
	ELF_strtab_TYPE,				// Strings that represent the names associated with symbol table entries
	ELF_debug_abbrev_TYPE,			// Abbreviations used in the .debug_info section
	ELF_debug_addr_TYPE,			// Contain a list of relocated addresses, one for each reference needed
	ELF_debug_aranges_TYPE,			// Lookup table for mapping addresses to compilation units
	ELF_debug_frame_TYPE,			// Call frame information
	ELF_debug_info_TYPE,			// Core DWARF information section
	ELF_debug_line_TYPE,			// Line number information
	ELF_debug_loc_TYPE,				// Location lists used in the DW_AT_location attributes
	ELF_debug_loclists_TYPE,		// DWARF5: Replace location list (.debug_loc) section
	ELF_debug_macinfo_TYPE,			// Macro information
	ELF_debug_pubnames_TYPE,		// Lookup table for mapping object and function names to compilation units
	ELF_debug_pubtypes_TYPE,		// Lookup table for mapping type names to compilation units
	ELF_debug_ranges_TYPE,			// Address ranges used in the DW_AT_ranges attributes
	ELF_debug_rnglists_TYPE,		// DWARF5: Replace range list (.debug_ranges) section
	ELF_debug_str_TYPE,				// String table used in .debug_info
	ELF_debug_types_TYPE,			// Type descriptions
	ELF_END_TYPE
}ELFSECTIONTYPE;


// Internal manager
extern void	ELFManager_Init(void);
extern bool	ELFManager_DwarfInit(Elf *PtrElfMem, struct stat FileElfInfo);
extern Elf *ELFManager_MemOpen(char *PtrELFExe, size_t Size);
extern int	ELFManager_MemEnd(void);
extern void	ELFManager_Reset(void);
extern void	ELFManager_Close(void);
extern bool ELFManager_AddTab(void *Ptr, size_t type);
extern void	*ELFManager_ExeCopy(void *src, size_t size);

// Sections manager
extern size_t ELFManager_GetSectionType(char *SectionName);

// Symbols manager
extern size_t ELFManager_GetAdrFromSymbolName(char *SymbolName);
extern char *ELFManager_GetSymbolnameFromAdr(size_t Adr);

// Functions manager
extern char *ELFManager_GetFunctionName(size_t Adr);


#endif	// __ELFMANAGER_H__
