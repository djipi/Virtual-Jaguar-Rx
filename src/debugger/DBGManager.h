

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

//
typedef enum {
	DBG_TAG_TYPE_structure = 0x1,					// structure
	DBG_TAG_TYPE_pointer = 0x2,						// pointer
	DBG_TAG_TYPE_subrange = 0x4,					// (subrange_type?)
	DBG_TAG_TYPE_array = 0x8,						// array type
	DBG_TAG_TYPE_const = 0x10,						// const type
	DBG_TAG_TYPE_typedef = 0x20,					// typedef
	DBG_TAG_TYPE_enumeration_type =	0x40,			// enumeration
	DBG_TAG_TYPE_subroutine_type = 0x80				// subroutine
}DBGTAGTYPE;

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

// Encoding based in the DW_OP_... list from the dwarf.h
typedef enum {
	DBG_NO_OP,
	DBG_OP_addr = 0x03,
	DBG_OP_deref = 0x06,
	DBG_OP_const1u = 0x08,
	DBG_OP_const1s = 0x09,
	DBG_OP_const2u = 0x0a,
	DBG_OP_const2s = 0x0b,
	DBG_OP_const4u = 0x0c,
	DBG_OP_const4s = 0x0d,
	DBG_OP_const8u = 0x0e,
	DBG_OP_const8s = 0x0f,
	DBG_OP_constu = 0x10,
	DBG_OP_consts = 0x11,
	DBG_OP_dup = 0x12,
	DBG_OP_drop = 0x13,
	DBG_OP_over = 0x14,
	DBG_OP_pick = 0x15,
	DBG_OP_swap = 0x16,
	DBG_OP_rot = 0x17,
	DBG_OP_xderef = 0x18,
	DBG_OP_abs = 0x19,
	DBG_OP_and = 0x1a,
	DBG_OP_div = 0x1b,
	DBG_OP_minus = 0x1c,
	DBG_OP_mod = 0x1d,
	DBG_OP_mul = 0x1e,
	DBG_OP_neg = 0x1f,
	DBG_OP_not = 0x20,
	DBG_OP_or = 0x21,
	DBG_OP_plus = 0x22,
	DBG_OP_plus_uconst = 0x23,
	DBG_OP_shl = 0x24,
	DBG_OP_shr = 0x25,
	DBG_OP_shra = 0x26,
	DBG_OP_xor = 0x27,
	DBG_OP_bra = 0x28,
	DBG_OP_eq = 0x29,
	DBG_OP_ge = 0x2a,
	DBG_OP_gt = 0x2b,
	DBG_OP_le = 0x2c,
	DBG_OP_lt = 0x2d,
	DBG_OP_ne = 0x2e,
	DBG_OP_skip = 0x2f,
	DBG_OP_lit0 = 0x30,
	DBG_OP_lit1 = 0x31,
	DBG_OP_lit2 = 0x32,
	DBG_OP_lit3 = 0x33,
	DBG_OP_lit4 = 0x34,
	DBG_OP_lit5 = 0x35,
	DBG_OP_lit6 = 0x36,
	DBG_OP_lit7 = 0x37,
	DBG_OP_lit8 = 0x38,
	DBG_OP_lit9 = 0x39,
	DBG_OP_lit10 = 0x3a,
	DBG_OP_lit11 = 0x3b,
	DBG_OP_lit12 = 0x3c,
	DBG_OP_lit13 = 0x3d,
	DBG_OP_lit14 = 0x3e,
	DBG_OP_lit15 = 0x3f,
	DBG_OP_lit16 = 0x40,
	DBG_OP_lit17 = 0x41,
	DBG_OP_lit18 = 0x42,
	DBG_OP_lit19 = 0x43,
	DBG_OP_lit20 = 0x44,
	DBG_OP_lit21 = 0x45,
	DBG_OP_lit22 = 0x46,
	DBG_OP_lit23 = 0x47,
	DBG_OP_lit24 = 0x48,
	DBG_OP_lit25 = 0x49,
	DBG_OP_lit26 = 0x4a,
	DBG_OP_lit27 = 0x4b,
	DBG_OP_lit28 = 0x4c,
	DBG_OP_lit29 = 0x4d,
	DBG_OP_lit30 = 0x4e,
	DBG_OP_lit31 = 0x4f,
	DBG_OP_reg0 = 0x50,
	DBG_OP_reg1 = 0x51,
	DBG_OP_reg2 = 0x52,
	DBG_OP_reg3 = 0x53,
	DBG_OP_reg4 = 0x54,
	DBG_OP_reg5 = 0x55,
	DBG_OP_reg6 = 0x56,
	DBG_OP_reg7 = 0x57,
	DBG_OP_reg8 = 0x58,
	DBG_OP_reg9 = 0x59,
	DBG_OP_reg10 = 0x5a,
	DBG_OP_reg11 = 0x5b,
	DBG_OP_reg12 = 0x5c,
	DBG_OP_reg13 = 0x5d,
	DBG_OP_reg14 = 0x5e,
	DBG_OP_reg15 = 0x5f,
	DBG_OP_reg16 = 0x60,
	DBG_OP_reg17 = 0x61,
	DBG_OP_reg18 = 0x62,
	DBG_OP_reg19 = 0x63,
	DBG_OP_reg20 = 0x64,
	DBG_OP_reg21 = 0x65,
	DBG_OP_reg22 = 0x66,
	DBG_OP_reg23 = 0x67,
	DBG_OP_reg24 = 0x68,
	DBG_OP_reg25 = 0x69,
	DBG_OP_reg26 = 0x6a,
	DBG_OP_reg27 = 0x6b,
	DBG_OP_reg28 = 0x6c,
	DBG_OP_reg29 = 0x6d,
	DBG_OP_reg30 = 0x6e,
	DBG_OP_reg31 = 0x6f,
	DBG_OP_breg0 = 0x70,
	DBG_OP_breg1 = 0x71,
	DBG_OP_breg2 = 0x72,
	DBG_OP_breg3 = 0x73,
	DBG_OP_breg4 = 0x74,
	DBG_OP_breg5 = 0x75,
	DBG_OP_breg6 = 0x76,
	DBG_OP_breg7 = 0x77,
	DBG_OP_breg8 = 0x78,
	DBG_OP_breg9 = 0x79,
	DBG_OP_breg10 = 0x7a,
	DBG_OP_breg11 = 0x7b,
	DBG_OP_breg12 = 0x7c,
	DBG_OP_breg13 = 0x7d,
	DBG_OP_breg14 = 0x7e,
	DBG_OP_breg15 = 0x7f,
	DBG_OP_breg16 = 0x80,
	DBG_OP_breg17 = 0x81,
	DBG_OP_breg18 = 0x82,
	DBG_OP_breg19 = 0x83,
	DBG_OP_breg20 = 0x84,
	DBG_OP_breg21 = 0x85,
	DBG_OP_breg22 = 0x86,
	DBG_OP_breg23 = 0x87,
	DBG_OP_breg24 = 0x88,
	DBG_OP_breg25 = 0x89,
	DBG_OP_breg26 = 0x8a,
	DBG_OP_breg27 = 0x8b,
	DBG_OP_breg28 = 0x8c,
	DBG_OP_breg29 = 0x8d,
	DBG_OP_breg30 = 0x8e,
	DBG_OP_breg31 = 0x8f,
	DBG_OP_regx = 0x90,
	DBG_OP_fbreg = 0x91,
	DBG_OP_bregx = 0x92,
	DBG_OP_piece = 0x93,
	DBG_OP_deref_size = 0x94,
	DBG_OP_xderef_size = 0x95,
	DBG_OP_nop = 0x96,
	DBG_OP_push_object_address = 0x97,		/* DWARF3 */
	DBG_OP_call2 = 0x98,					/* DWARF3 */
	DBG_OP_call4 = 0x99,					/* DWARF3 */
	DBG_OP_call_ref = 0x9a,					/* DWARF3 */
	DBG_OP_form_tls_address = 0x9b,			/* DWARF3f */
	DBG_OP_call_frame_cfa = 0x9c,			/* DWARF3f */
	DBG_OP_bit_piece = 0x9d,				/* DWARF3f */
	DBG_OP_implicit_value = 0x9e,			/* DWARF4 */
	DBG_OP_stack_value = 0x9f,				/* DWARF4 */
	DBG_END_OP
}
DBGOP;


// Internal manager
extern void	DBGManager_Init(void);
extern void	DBGManager_SetType(size_t DBGTypeSet);
extern size_t DBGManager_GetType(void);
extern void	DBGManager_Reset(void);
extern void	DBGManager_Close(void);

// General manager
extern char	*DBGManager_GetSymbolNameFromAdr(size_t Adr);
extern char	*DBGManager_GetFullSourceFilenameFromAdr(size_t Adr, bool *Error);
extern size_t DBGManager_GetNumLineFromAdr(size_t Adr, size_t Tag);
extern char *DBGManager_GetLineSrcFromAdr(size_t Adr, size_t Tag);
extern char *DBGManager_GetLineSrcFromAdrNumLine(size_t Adr, size_t NumLine);
extern char *DBGManager_GetLineSrcFromNumLineBaseAdr(size_t Adr, size_t NumLine);
extern size_t DBGManager_GetAdrFromSymbolName(char *SymbolName);
extern char *DBGManager_GetFunctionName(size_t Adr);
extern char *DBGManager_GetVariableValueFromAdr(size_t Adr, size_t TypeEncoding, size_t TypeByteSize);
extern size_t DBGManager_GetNbFullSourceFilename(void);
extern char *DBGManager_GetNumFullSourceFilename(size_t Index);

// Global variables manager
extern size_t DBGManager_GetNbGlobalVariables(void);
extern char *DBGManager_GetGlobalVariableName(size_t Index);
extern size_t DBGManager_GetGlobalVariableTypeEncoding(size_t Index);
extern char *DBGManager_GetGlobalVariableTypeName(size_t Index);
extern size_t DBGManager_GetGlobalVariableTypeByteSize(size_t Index);
extern size_t DBGManager_GetGlobalVariableAdr(size_t Index);
extern size_t DBGManager_GetGlobalVariableAdrFromName(char *VariableName);
extern char *DBGManager_GetGlobalVariableValue(size_t Index);
extern size_t DBGManager_GetGlobalVariableTypeTag(size_t Index);

// Local variables manager
extern size_t DBGManager_GetNbLocalVariables(size_t Adr);
extern char *DBGManager_GetLocalVariableName(size_t Adr, size_t Index);
extern size_t DBGManager_GetLocalVariableTypeEncoding(size_t Adr, size_t Index);
extern char *DBGManager_GetLocalVariableTypeName(size_t Adr, size_t Index);
extern size_t DBGManager_GetLocalVariableTypeByteSize(size_t Adr, size_t Index);
extern size_t DBGManager_GetLocalVariableTypeTag(size_t Adr, size_t Index);
extern size_t DBGManager_GetLocalVariableOp(size_t Adr, size_t Index);
extern int DBGManager_GetLocalVariableOffset(size_t Adr, size_t Index);


#endif	// __DBGMANAGER_H__
