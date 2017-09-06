

#ifndef __DWARFMANAGER_H__
#define __DWARFMANAGER_H__


// 
extern bool	DWARFManager_Reset(void);
extern bool	DWARFManager_Close(void);
extern void	DWARFManager_Init(void);
extern int DWARFManager_ElfInit(Elf *ElfPtr);

// 
extern char	*DWARFManager_GetFullSourceFilenameFromAdr(size_t Adr, bool *Error);
extern size_t DWARFManager_GetNumLineFromAdr(size_t Adr, size_t Tag);
extern char	*DWARFManager_GetSymbolnameFromAdr(size_t Adr);
extern char *DWARFManager_GetLineSrcFromAdr(size_t Adr, size_t Tag);
extern char *DWARFManager_GetLineSrcFromAdrNumLine(size_t Adr, size_t NumLine);
extern char *DWARFManager_GetLineSrcFromNumLineBaseAdr(size_t Adr, size_t NumLine);

// External variables manager
extern size_t DWARFManager_GetNbExternalVariables(void);
extern char *DWARFManager_GetExternalVariableName(size_t Index);
extern size_t DWARFManager_GetExternalVariableTypeEncoding(size_t Index);
extern char *DWARFManager_GetExternalVariableTypeName(size_t Index);
extern size_t DWARFManager_GetExternalVariableTypeByteSize(size_t Index);
extern size_t DWARFManager_GetExternalVariableAdr(size_t Index);
extern size_t DWARFManager_GetExternalVariableAdrFromName(char *VariableName);
extern size_t DWARFManager_GetExternalVariableTypeTag(size_t Index);


#endif	// __DWARFMANAGER_H__
