//
// DBGManager.cpp: Debugger information manager
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM  12/21/2016  Created this file
// JPM              Various efforts to set the ELF format support
// JPM              Various efforts to set the DWARF format support

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "libelf/libelf.h"
#include "libelf/gelf.h"
#include "log.h"
#include "ELFManager.h"
#include "DwarfManager.h"
#include "DBGManager.h"
#include "HWLABELManager.h"
#include "settings.h"
#include "memory.h"


// 
char *DBGManager_GetVariableValueFromAdr(uint32_t Adr, uint32_t TypeEncoding, uint32_t TypeByteSize);


//
struct Value
{
	union
	{
		char C[10];
		double D;
		float F;
		int32_t SI;
		int64_t SL;
		uint32_t UI;
		uint64_t UL;
	};
}S_Value;


// Common debugger variables
size_t	DBGType;
char value[1000];


// Common debugger initialisation
void DBGManager_Init(void)
{
	DBGType = DBG_NO_TYPE;
	ELFManager_Init();
	DWARFManager_Init();
}


// Common debugger reset
void DBGManager_Reset(void)
{
	if ((DBGType & DBG_DWARF))
	{
		DWARFManager_Reset();
	}

	if ((DBGType & DBG_ELF))
	{
		ELFManager_Reset();
	}

	//DBGType = vjs.displayHWlabels ? DBG_HWLABEL : DBG_NO_TYPE;
	DBGType = DBG_NO_TYPE;
}


// Common debugger set
void DBGManager_SetType(size_t DBGTypeSet)
{
	DBGType |= DBGTypeSet;
}


// Common debugger close
void DBGManager_Close(void)
{
	if ((DBGType & DBG_DWARF))
	{
		DWARFManager_Close();
	}

	if ((DBGType & DBG_ELF))
	{
		ELFManager_Close();
	}
}


// Get source filename based on the memeory address
// return NULL if no source filename
char *DBGManager_GetFullSourceFilenameFromAdr(size_t Adr, bool *Error)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetFullSourceFilenameFromAdr(Adr, Error);
	}
	else
	{
		return	NULL;
	}
}


// Get number of external variables
// Return 0 if none has been found
size_t DBGManager_GetNbExternalVariables(void)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetNbExternalVariables();
	}
	else
	{
		return	0;
	}
}


//
size_t DBGManager_GetAdrFromSymbolName(char *SymbolName)
{
	if ((DBGType & DBG_ELF))
	{
		return ELFManager_GetAdrFromSymbolName(SymbolName);
	}
	else
	{
		return 0;
	}
}


// Get external variable's Address based on his Name
// Return found Address
// Return NULL if no Address has been found
size_t DBGManager_GetExternalVariableAdrFromName(char *VariableName)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetExternalVariableAdrFromName(VariableName);
	}
	else
	{
		return 0;
	}
}


//
size_t DBGManager_GetExternalVariableTypeTag(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetExternalVariableTypeTag(Index);
	}
	else
	{
		return	0;
	}
}


// Get external variable's type name based on his Index
// Return type name's text pointer found
// Return NULL if no type name has been found
char *DBGManager_GetExternalVariableTypeName(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetExternalVariableTypeName(Index);
	}
	else
	{
		return	NULL;
	}
}


// Get external variable's Address based on his Index
// Return the Address found
// Return 0 if no Address has been found
size_t DBGManager_GetExternalVariableAdr(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetExternalVariableAdr(Index);
	}
	else
	{
		return	0;
	}
}


// Get external variable's type byte size based on his Index
// Return the type's byte size found
// Return 0 if no type's byte size has been found
size_t DBGManager_GetExternalVariableTypeByteSize(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetExternalVariableTypeByteSize(Index);
	}
	else
	{
		return	0;
	}
}


// Get external variable's type encoding based on his Index
// Return the type encoding found
// Return 0 if no type encoding has been found
size_t DBGManager_GetExternalVariableTypeEncoding(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetExternalVariableTypeEncoding(Index);
	}
	else
	{
		return	0;
	}
}


// Get external variable value based on his Index
// Return value as a text pointer
// Note: Pointer may point on a 0 lenght text
char *DBGManager_GetExternalVariableValue(size_t Index)
{
	uint32_t Adr = 0;
	uint32_t TypeEncoding = DBG_NO_TYPEENCODING;
	uint32_t TypeByteSize = 0;

	if ((DBGType & DBG_ELFDWARF))
	{
		Adr = DWARFManager_GetExternalVariableAdr(Index);
		TypeEncoding = DWARFManager_GetExternalVariableTypeEncoding(Index);
		TypeByteSize = DWARFManager_GetExternalVariableTypeByteSize(Index);
	}

	return DBGManager_GetVariableValueFromAdr(Adr, TypeEncoding, TypeByteSize);
}


// Get variable value based on his Adresse, Encoding Type and Size
// Return value as a text pointer
// Note: Pointer may point on a 0 lenght text if Adress is NULL
char *DBGManager_GetVariableValueFromAdr(uint32_t Adr, uint32_t TypeEncoding, uint32_t TypeByteSize)
{
	Value V;
	char *Ptrvalue = value;

	value[0] = 0;

	if (Adr)
	{
		memset(&V, 0, sizeof(Value));
#if 0
		for (uint32_t i = 0; i < TypeByteSize; i++)
			jaguarMainRAM[Adr + i] = 0;
			//jaguarMainRAM[Adr + i] = rand();
		jaguarMainRAM[Adr + TypeByteSize - 1] = 0x10;
#endif
#if 1
		for (uint32_t i = 0, j = TypeByteSize; i < TypeByteSize; i++, j--)
		{
			V.C[i] = jaguarMainRAM[Adr + j - 1];
		}
#endif

		switch (TypeEncoding)
		{
		case DBG_ATE_address:
			break;

		case DBG_ATE_boolean:
			break;

		case DBG_ATE_complex_float:
			break;

		case DBG_ATE_float:
			switch (TypeByteSize)
			{
			case 4:
				sprintf(value, "%F", V.F);
				break;

			case 8:
				//V.D = (double)jaguarMainRAM[Adr];
				//sprintf(value, "%10.10F", V.D);
				sprintf(value, "%F", V.D);
				break;

			default:
				break;
			}
			break;

		case DBG_ATE_signed:
			switch (TypeByteSize)
			{
			case 4:
				sprintf(value, "%i", V.SI);
				break;

			case 8:
				sprintf(value, "%i", V.SL);
				break;

			default:
				break;
			}
			break;

		case DBG_ATE_signed_char:
			break;

		case DBG_ATE_unsigned:
			switch (TypeByteSize)
			{
			case 4:
				sprintf(value, "%u", V.UI);
				break;

			case 8:
				sprintf(value, "%u", V.UL);
				break;

			default:
				break;
			}
			break;

		case DBG_ATE_unsigned_char:
			break;

		case DBG_ATE_ptr:
			switch (TypeByteSize)
			{
			case 4:
				sprintf(value, "0x%06x", V.UI);
				break;

			default:
				break;
			}

		default:
			break;
		}
	}

	return Ptrvalue;
}


// Get external variable name based on his Index
// Return variable name's text pointer found
// Return NULL if no variable name has been found
char *DBGManager_GetExternalVariableName(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetExternalVariableName(Index);
	}
	else
	{
		return	NULL;
	}
}


// Get line number from address and his tag
// Return line number on the symbol name found
// Return 0 if no symbol name has been found
size_t DBGManager_GetNumLineFromAdr(size_t Adr, size_t Tag)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetNumLineFromAdr(Adr, Tag);
	}
	else
	{
		return	0;
	}
}


// Get symbol name from address
// Return text pointer on the symbol name found
// Return NULL if no symbol name has been found
char *DBGManager_GetSymbolnameFromAdr(size_t Adr)
{
	char *Symbolname;

	//if ((DBGType & DBG_HWLABEL) || vjs.displayHWlabels)
	if (vjs.displayHWlabels)
	{
		Symbolname = HWLABELManager_GetSymbolnameFromAdr(Adr);
	}
	else
	{
		Symbolname = NULL;
	}
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to set the DBG_HWLABEL in DBGType instead to use the setting value !!!")
#else
	#warning "!!! Need to set the DBG_HWLABEL in DBGType instead to use the setting value !!!"
#endif // _MSC_VER

	if (Symbolname == NULL)
	{
		if ((DBGType & DBG_ELFDWARF))
		{
			Symbolname = DWARFManager_GetSymbolnameFromAdr(Adr);
		}

		if ((DBGType & DBG_ELF) && (Symbolname == NULL))
		{
			Symbolname = ELFManager_GetSymbolnameFromAdr(Adr);
		}
	}

	return	Symbolname;
}


// Get source line based on the Address and his Tag
// Return text pointer on the source line found
// Return NULL if no source line has been found
char *DBGManager_GetLineSrcFromAdr(size_t Adr, size_t Tag)
{
	char *Symbolname = NULL;

	if ((DBGType & DBG_ELFDWARF))
	{
		Symbolname = DWARFManager_GetLineSrcFromAdr(Adr, Tag);
	}

	return	Symbolname;
}


// Get text line from source based on address and num line (starting by 1)
// Return NULL if no text line has been found
char *DBGManager_GetLineSrcFromAdrNumLine(size_t Adr, size_t NumLine)
{
	char *Symbolname = NULL;

	if ((DBGType & DBG_ELFDWARF))
	{
		Symbolname = DWARFManager_GetLineSrcFromAdrNumLine(Adr, NumLine);
	}

	return	Symbolname;
}


// Get text line from source based on address and num line (starting by 1)
// Return NULL if no text line has been found
char *DBGManager_GetLineSrcFromNumLineBaseAdr(size_t Adr, size_t NumLine)
{
	char *Symbolname = NULL;

	if ((DBGType & DBG_ELFDWARF))
	{
		Symbolname = DWARFManager_GetLineSrcFromNumLineBaseAdr(Adr, NumLine);
	}

	return	Symbolname;
}

