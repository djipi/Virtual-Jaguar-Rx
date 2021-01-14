

#ifndef __DWARFMANAGER_H__
#define __DWARFMANAGER_H__


// Definition for the DWARF status of each source file
typedef enum
{
	DWARFSTATUS_OK = 0x0,
	DWARFSTATUS_OUTDATEDFILE = 0x1,
	DWARFSTATUS_NOFILE = 0x2,
	DWARFSTATUS_NOFILEINFO = 0x4,
	DWARFSTATUS_UNKNOWN = 0xff
}DWARFstatus;

// Internal manager
extern bool	DWARFManager_Reset(void);
extern bool	DWARFManager_Close(void);
extern void	DWARFManager_Init(void);
extern int DWARFManager_ElfInit(Elf *ElfPtr, struct stat FileElfInfo);
extern void DWARFManager_Set(size_t NbPathsInList, char **PtrListPaths);
extern size_t DWARFManager_GetNbSources(void);

// General manager
extern char *DWARFManager_GetFunctionName(size_t Adr);
extern size_t DWARFManager_GetSrcLanguageFromIndex(size_t Index);

// Source text files manager
extern char	*DWARFManager_GetFullSourceFilenameFromAdr(size_t Adr, DWARFstatus *Status);
extern char *DWARFManager_GetNumFullSourceFilename(size_t Index);
extern char *DWARFManager_GetNumSourceFilename(size_t Index);

// Symbols manager
extern char	*DWARFManager_GetSymbolnameFromAdr(size_t Adr);

// Source text lines manager
extern size_t DWARFManager_GetNumLineFromAdr(size_t Adr, size_t Tag);
extern char *DWARFManager_GetLineSrcFromAdr(size_t Adr, size_t Tag);
extern char *DWARFManager_GetLineSrcFromAdrNumLine(size_t Adr, size_t NumLine);
extern char *DWARFManager_GetLineSrcFromNumLineBaseAdr(size_t Adr, size_t NumLine);
extern char **DWARFManager_GetSrcListPtrFromIndex(size_t Index, bool Used);
extern size_t DWARFManager_GetSrcNbListPtrFromIndex(size_t Index, bool Used);
extern size_t *DWARFManager_GetSrcNumLinesPtrFromIndex(size_t Index, bool Used);

// Global variables manager
extern size_t DWARFManager_GetNbGlobalVariables(void);
extern char *DWARFManager_GetGlobalVariableName(size_t Index);
extern size_t DWARFManager_GetGlobalVariableTypeEncoding(size_t Index);
extern char *DWARFManager_GetGlobalVariableTypeName(size_t Index);
extern size_t DWARFManager_GetGlobalVariableTypeByteSize(size_t Index);
extern size_t DWARFManager_GetGlobalVariableAdr(size_t Index);
extern size_t DWARFManager_GetGlobalVariableAdrFromName(char *VariableName);
extern size_t DWARFManager_GetGlobalVariableTypeTag(size_t Index);

// Local variables manager
extern size_t DWARFManager_GetNbLocalVariables(size_t Adr);
extern char *DWARFManager_GetLocalVariableName(size_t Adr, size_t Index);
extern size_t DWARFManager_GetLocalVariableTypeEncoding(size_t Adr, size_t Index);
extern char *DWARFManager_GetLocalVariableTypeName(size_t Adr, size_t Index);
extern size_t DWARFManager_GetLocalVariableTypeByteSize(size_t Adr, size_t Index);
extern size_t DWARFManager_GetLocalVariableTypeTag(size_t Adr, size_t Index);
extern size_t DWARFManager_GetLocalVariableOp(size_t Adr, size_t Index);
extern int DWARFManager_GetLocalVariableOffset(size_t Adr, size_t Index);


#endif	// __DWARFMANAGER_H__
