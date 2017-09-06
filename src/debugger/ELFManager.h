

#ifndef __ELFMANAGER_H__
#define __ELFMANAGER_H__


typedef enum {
	ELF_NO_TYPE = -1,
	ELF_NULL_TYPE = 0,
	ELF_text_TYPE,
	ELF_rodata_TYPE,
	ELF_data_TYPE,
	ELF_bss_TYPE,
	ELF_heap_TYPE,
	ELF_debug_aranges_TYPE,
	ELF_debug_info_TYPE,
	ELF_debug_abbrev_TYPE,
	ELF_debug_line_TYPE,
	ELF_debug_frame_TYPE,
	ELF_debug_str_TYPE,
	ELF_debug_loc_TYPE,
	ELF_debug_ranges_TYPE,
	ELF_comment_TYPE,
	ELF_shstrtab_TYPE,
	ELF_symtab_TYPE,
	ELF_strtab_TYPE,
	ELF_END_TYPE
}ELFSECTIONTYPE;


extern void	ELFManager_Init(void);
extern bool	ELFManager_DwarfInit(Elf *PtrElfMem);
extern Elf *ELFManager_MemOpen(char *PtrELFExe, size_t Size);
extern int	ELFManager_MemEnd(void);
extern void	ELFManager_Reset(void);
extern void	ELFManager_Close(void);
extern bool ELFManager_AddTab(void *Ptr, size_t type);
extern char *ELFManager_GetSymbolnameFromAdr(size_t Adr);
extern void	*ELFManager_ExeCopy(void *src, size_t size);
extern size_t ELFManager_GetSectionType(char *SectionName);
extern size_t ELFManager_GetAdrFromSymbolName(char *SymbolName);


#endif	// __ELFMANAGER_H__
