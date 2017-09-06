

#ifndef __DBGMANAGER_H__
#define __DBGMANAGER_H__


typedef enum {
	DBG_NO_TYPE = 0x0,
	DBG_ELF = 0x1,
	DBG_DWARF = 0x2,
	DBG_ELFDWARF = 0x4,
	DBG_HWLABEL = 0x8,
	DBG_END_TYPE
}DBGTYPE;

// Tag based in the DW_TAG_... list from the dwarf.h
typedef enum {
	DBG_NO_TAG = 0x0,
	DBG_TAG_pointer_type = 0x0f,
	DBG_TAG_compile_unit = 0x11,
	DBG_TAG_base_type = 0x24,
	DBG_TAG_subprogram = 0x2e,
	DBG_END_TAG
}DBGTAG;

// Encoding based in the DW_ATE_... list from the dwarf.h
// Except for the DBG_ATE_ptr
typedef enum {
	DBG_NO_TYPEENCODING,
	DBG_ATE_address = 0x1,					// linear machine address
	DBG_ATE_boolean = 0x2,					// true or false
	DBG_ATE_complex_float = 0x3,			// complex floating-point number
	DBG_ATE_float = 0x4,					// floating-point number
	DBG_ATE_signed = 0x5,					// signed binary integer
	DBG_ATE_signed_char = 0x6,				// signed character
	DBG_ATE_unsigned = 0x7,					// unsigned binary integer
	DBG_ATE_unsigned_char = 0x8,			// unsigned character
	DBG_ATE_imaginary_float = 0x9,			/* DWARF3 */
	DBG_ATE_packed_decimal = 0xa,			/* DWARF3f */
	DBG_ATE_numeric_string = 0xb,			/* DWARF3f */
	DBG_ATE_edited = 0xc,					/* DWARF3f */
	DBG_ATE_signed_fixed = 0xd,				/* DWARF3f */
	DBG_ATE_unsigned_fixed = 0xe,			/* DWARF3f */
	DBG_ATE_decimal_float = 0xf,			/* DWARF3f */
	DBG_ATE_ptr = 0x10,						// Specific to DBG Manager to represent pointer type
	DBG_END_TYPEENCODING
}DBGTYPEENCODING;


//
extern void	DBGManager_Init(void);
extern void	DBGManager_SetType(int DBGTypeSet);
extern void	DBGManager_Reset(void);
extern void	DBGManager_Close(void);

//
extern char	*DBGManager_GetSymbolnameFromAdr(size_t Adr);
extern char	*DBGManager_GetFullSourceFilenameFromAdr(size_t Adr, bool *Error);
extern size_t DBGManager_GetNumLineFromAdr(size_t Adr, size_t Tag);
extern char *DBGManager_GetLineSrcFromAdr(size_t Adr, size_t Tag);
extern char *DBGManager_GetLineSrcFromAdrNumLine(size_t Adr, size_t NumLine);
extern char *DBGManager_GetLineSrcFromNumLineBaseAdr(size_t Adr, size_t NumLine);

// External variables manager
extern size_t DBGManager_GetNbExternalVariables(void);
extern char *DBGManager_GetExternalVariableName(size_t Index);
extern size_t DBGManager_GetExternalVariableTypeEncoding(size_t Index);
extern char *DBGManager_GetExternalVariableTypeName(size_t Index);
extern size_t DBGManager_GetExternalVariableTypeByteSize(size_t Index);
extern size_t DBGManager_GetExternalVariableAdr(size_t Index);
extern size_t DBGManager_GetExternalVariableAdrFromName(char *VariableName);
extern size_t DBGManager_GetAdrFromSymbolName(char *SymbolName);
extern char *DBGManager_GetExternalVariableValue(size_t Index);
extern size_t DBGManager_GetExternalVariableTypeTag(size_t Index);


#endif	// __DBGMANAGER_H__
