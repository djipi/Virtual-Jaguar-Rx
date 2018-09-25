

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
extern char *DWARFManager_GetFunctionName(size_t Adr);
extern size_t DWARFManager_GetNbFullSourceFilename(void);
extern char *DWARFManager_GetNumFullSourceFilename(size_t Index);

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
