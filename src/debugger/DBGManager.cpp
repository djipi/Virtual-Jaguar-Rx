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
// JPM  09/15/2018  Support the unsigned char
// JPM   Oct./2018  Cosmetic changes, added source file search paths, and ELF function name
// JPM   Aug./2019  Added new functions mainly for source text lines
// JPM  Sept./2019  Support the unsigned/signed short type
//

// To Do
// To think about unique format to handle variations from ELF, DWARF, etc.
//


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
struct Value
{
	union
	{
		char Ct[10];
		char C;
		bool B;
		double D;
		float F;
		int16_t SS;
		int32_t SI;
		int64_t SL;
		uint16_t US;
		uint32_t UI;
		uint64_t UL;
	};
}S_Value;


//
void DBGManager_SourceFileSearchPathsInit(void);
void DBGManager_SourceFileSearchPathsReset(void);
void DBGManager_SourceFileSearchPathsClose(void);


// Common debugger variables
size_t	DBGType;
char value[1000];
size_t NbSFSearchPaths;
char **SourceFileSearchPaths;


// Init the source file search paths
void DBGManager_SourceFileSearchPathsInit(void)
{
	NbSFSearchPaths = 0;
	SourceFileSearchPaths = NULL;
}


// Set the source file search paths
// Create individual path for each one provided in the list (separate with ';')
void DBGManager_SourceFileSearchPathsSet(char *ListPaths)
{
	// Check presence of a previous list
	if (NbSFSearchPaths)
	{
		// Reset previous list
		DBGManager_SourceFileSearchPathsReset();
	}

	// Check if there is a paths list
	if (strlen(ListPaths))
	{
		// Get number of paths
		char *Ptr = ListPaths;
		while(*Ptr)
		{
			while (*Ptr && (*Ptr++ != ';'));
			{
				NbSFSearchPaths++;
			}
		}

		// Isolate each search path
		SourceFileSearchPaths = (char **)calloc(NbSFSearchPaths, sizeof(char *));
		size_t i = 0;
		Ptr = ListPaths;

		while (*Ptr)
		{
			// Search the path separator (';')
			char *Ptr1 = Ptr;
			while (*Ptr && (*Ptr++ != ';'));

			// Copy the inidividual search path
			SourceFileSearchPaths[i] = (char *)calloc(1, (Ptr - Ptr1) + 1);
			strncpy(SourceFileSearchPaths[i], Ptr1, (Ptr - Ptr1));
			if (SourceFileSearchPaths[i][strlen(SourceFileSearchPaths[i]) - 1] == ';')
			{
				SourceFileSearchPaths[i][strlen(SourceFileSearchPaths[i]) - 1] = 0;
			}
			i++;
		}
	}

	DWARFManager_Set(NbSFSearchPaths, SourceFileSearchPaths);
}


// Reset the source file search paths
void DBGManager_SourceFileSearchPathsReset(void)
{
	// Free each path
	while (NbSFSearchPaths)
	{
		free(SourceFileSearchPaths[--NbSFSearchPaths]);
	}

	// Free the pointers list
	free(SourceFileSearchPaths);
	SourceFileSearchPaths = NULL;
}


// Close the source file search paths
void DBGManager_SourceFileSearchPathsClose(void)
{
	DBGManager_SourceFileSearchPathsReset();
}


// Common debugger initialisation
void DBGManager_Init(void)
{
	// DBG initialisations
	DBGType = DBG_NO_TYPE;
	DBGManager_SourceFileSearchPathsInit();

	// ELF initialisation 
	ELFManager_Init();
	// DWARF initialisation
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

	DBGManager_SourceFileSearchPathsClose();
	DBGType = DBG_NO_TYPE;
}


// Common debugger set
void DBGManager_SetType(size_t DBGTypeSet)
{
	DBGType |= DBGTypeSet;
}


// Get debugger type
size_t DBGManager_GetType(void)
{
	return DBGType;
}


// Get source filename based on the memeory address
// return NULL if no source filename
char *DBGManager_GetFullSourceFilenameFromAdr(size_t Adr, DBGstatus *Status)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetFullSourceFilenameFromAdr(Adr, (DWARFstatus *)Status);
	}
	else
	{
		return	NULL;
	}
}


// Get number of local variables
// Return 0 if none has been found
size_t DBGManager_GetNbLocalVariables(size_t Adr)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetNbLocalVariables(Adr);
	}
	else
	{
		return	0;
	}
}


// Get number of global variables
// Return 0 if none has been found
size_t DBGManager_GetNbGlobalVariables(void)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetNbGlobalVariables();
	}
	else
	{
		return	0;
	}
}


// Get address from symbol name
// Return found address
// Return NULL if no symbol has been found
size_t DBGManager_GetAdrFromSymbolName(char *SymbolName)
{
	if (SymbolName)
	{
		if ((DBGType & DBG_ELF))
		{
			return ELFManager_GetAdrFromSymbolName(SymbolName);
		}
	}

	return 0;
}


// Get global variable's Address based on his Name
// Return found Address
// Return NULL if no Address has been found
size_t DBGManager_GetGlobalVariableAdrFromName(char *VariableName)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetGlobalVariableAdrFromName(VariableName);
	}
	else
	{
		return 0;
	}
}


// Get local variable's type encoding based on his address and Index
// Return the type encoding found
// Return 0 if no type encoding has been found
size_t DBGManager_GetLocalVariableTypeEncoding(size_t Adr, size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetLocalVariableTypeEncoding(Adr, Index);
	}
	else
	{
		return	0;
	}
}


//
int DBGManager_GetLocalVariableOffset(size_t Adr, size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetLocalVariableOffset(Adr, Index);
	}
	else
	{
		return	0;
	}
}


// Get local variable's type byte size based on his address and Index
// Return the type's byte size found
// Return 0 if no type's byte size has been found
size_t DBGManager_GetLocalVariableTypeByteSize(size_t Adr, size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetLocalVariableTypeByteSize(Adr, Index);
	}
	else
	{
		return	0;
	}
}


//
size_t DBGManager_GetLocalVariableTypeTag(size_t Adr, size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetLocalVariableTypeTag(Adr, Index);
	}
	else
	{
		return	0;
	}
}


//
size_t DBGManager_GetGlobalVariableTypeTag(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetGlobalVariableTypeTag(Index);
	}
	else
	{
		return	0;
	}
}


// Get global variable's type name based on his Index
// Return type name's text pointer found
// Return NULL if no type name has been found
char *DBGManager_GetGlobalVariableTypeName(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetGlobalVariableTypeName(Index);
	}
	else
	{
		return	NULL;
	}
}


// Get global variable's Address based on his Index
// Return the Address found
// Return 0 if no Address has been found
size_t DBGManager_GetGlobalVariableAdr(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetGlobalVariableAdr(Index);
	}
	else
	{
		return	0;
	}
}


// Get global variable's type byte size based on his Index
// Return the type's byte size found
// Return 0 if no type's byte size has been found
size_t DBGManager_GetGlobalVariableTypeByteSize(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetGlobalVariableTypeByteSize(Index);
	}
	else
	{
		return	0;
	}
}


// Get global variable's type encoding based on his Index
// Return the type encoding found
// Return 0 if no type encoding has been found
size_t DBGManager_GetGlobalVariableTypeEncoding(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetGlobalVariableTypeEncoding(Index);
	}
	else
	{
		return	0;
	}
}


// Get global variable value based on his Index
// Return value as a text pointer
// Note: Pointer may point on a 0 lenght text
char *DBGManager_GetGlobalVariableValue(size_t Index)
{
	size_t Adr = 0;
	size_t TypeEncoding = DBG_NO_TYPEENCODING;
	size_t TypeByteSize = 0;

	if ((DBGType & DBG_ELFDWARF))
	{
		Adr = DWARFManager_GetGlobalVariableAdr(Index);
		TypeEncoding = DWARFManager_GetGlobalVariableTypeEncoding(Index);
		TypeByteSize = DWARFManager_GetGlobalVariableTypeByteSize(Index);
	}

	return DBGManager_GetVariableValueFromAdr(Adr, TypeEncoding, TypeByteSize);
}


// Get variable value based on his Adresse, Encoding Type and Size
// Return value as a text pointer
// Note: Pointer may point on a 0 length text
char *DBGManager_GetVariableValueFromAdr(size_t Adr, size_t TypeEncoding, size_t TypeByteSize)
{
	Value V;
	char *Ptrvalue = value;

	value[0] = 0;

#if 0
	if (Adr)
#endif
	{
		memset(&V, 0, sizeof(Value));
#if 0
		for (uint32_t i = 0; i < TypeByteSize; i++)
			jaguarMainRAM[Adr + i] = 0;
			//jaguarMainRAM[Adr + i] = rand();
		jaguarMainRAM[Adr + TypeByteSize - 1] = 0x10;
#else
		for (size_t i = 0, j = TypeByteSize; i < TypeByteSize; i++, j--)
		{
			V.Ct[i] = jaguarMainRAM[Adr + j - 1];
		}
#endif
		switch (TypeEncoding)
		{
		case DBG_ATE_address:
			break;

		case DBG_ATE_boolean:
			sprintf(value, "%s", V.B ? "true" : "false");
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
			case 2:
				sprintf(value, "%i", V.SS);
				break;

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
			case 2:
				sprintf(value, "%u", V.US);
				break;

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
			sprintf(value, "%u", (unsigned int(V.C)));
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


// Get local variable's type name based on his Index
// Return type name's text pointer found
// Return NULL if no type name has been found
char *DBGManager_GetLocalVariableTypeName(size_t Adr, size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetLocalVariableTypeName(Adr, Index);
	}
	else
	{
		return	NULL;
	}
}


// Get local variable Op based on his Index
// Return variable Op's found
// Return 0 if no variable Op has been found
size_t DBGManager_GetLocalVariableOp(size_t Adr, size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetLocalVariableOp(Adr, Index);
	}
	else
	{
		return	0;
	}
}


// Get local variable name based on his Index
// Return variable name's text pointer found
// Return NULL if no variable name has been found
char *DBGManager_GetLocalVariableName(size_t Adr, size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetLocalVariableName(Adr, Index);
	}
	else
	{
		return	NULL;
	}
}


// Get global variable name based on his Index
// Return variable name's text pointer found
// Return NULL if no variable name has been found
char *DBGManager_GetGlobalVariableName(size_t Index)
{
	if ((DBGType & DBG_ELFDWARF))
	{
		return DWARFManager_GetGlobalVariableName(Index);
	}
	else
	{
		return	NULL;
	}
}


// Get function name from address
// Return function name found
// Return NULL if no function name has been found
char *DBGManager_GetFunctionName(size_t Adr)
{
	char *Symbolname = NULL;

	if ((DBGType & DBG_ELFDWARF))
	{
		Symbolname = DWARFManager_GetFunctionName(Adr);
	}

	if ((DBGType & DBG_ELF) && (Symbolname == NULL))
	{
		Symbolname = ELFManager_GetFunctionName(Adr);
	}

	return	Symbolname;
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
char *DBGManager_GetSymbolNameFromAdr(size_t Adr)
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
	char *TextLine = NULL;

	if ((DBGType & DBG_ELFDWARF))
	{
		TextLine = DWARFManager_GetLineSrcFromAdr(Adr, Tag);
	}

	return	TextLine;
}


// Get text line from source based on address and num line (starting from 1)
// Return NULL if no text line has been found
char *DBGManager_GetLineSrcFromAdrNumLine(size_t Adr, size_t NumLine)
{
	char *TextLine = NULL;

	if ((DBGType & DBG_ELFDWARF))
	{
		TextLine = DWARFManager_GetLineSrcFromAdrNumLine(Adr, NumLine);
	}

	return	TextLine;
}


// Get text line from source based on address and num line (starting from 1)
// Return NULL if no text line has been found
char *DBGManager_GetLineSrcFromNumLineBaseAdr(size_t Adr, size_t NumLine)
{
	char *TextLine = NULL;

	if ((DBGType & DBG_ELFDWARF))
	{
		TextLine = DWARFManager_GetLineSrcFromNumLineBaseAdr(Adr, NumLine);
	}

	return	TextLine;
}


// Get number of source code filenames
size_t DBGManager_GetNbSources(void)
{
	size_t Nbr = 0;

	if ((DBGType & DBG_ELFDWARF))
	{
		Nbr = DWARFManager_GetNbSources();
	}

	return Nbr;
}


// Get source code filename based on index
char *DBGManager_GetNumSourceFilename(size_t Index)
{
	char *SourceFilename = NULL;

	if ((DBGType & DBG_ELFDWARF))
	{
		SourceFilename = DWARFManager_GetNumSourceFilename(Index);
	}

	return	SourceFilename;
}


// Get source code filename based on index
char *DBGManager_GetNumFullSourceFilename(size_t Index)
{
	char *FullSourceFilename = NULL;

	if ((DBGType & DBG_ELFDWARF))
	{
		FullSourceFilename = DWARFManager_GetNumFullSourceFilename(Index);
	}

	return	FullSourceFilename;
}


// Get number of lines of texts source list from source index
size_t DBGManager_GetSrcNbListPtrFromIndex(size_t Index, bool Used)
{
	size_t NbListPtr = 0;

	if ((DBGType & DBG_ELFDWARF))
	{
		NbListPtr = DWARFManager_GetSrcNbListPtrFromIndex(Index, Used);
	}

	return	NbListPtr;
}


// Get pointer to the lines number list from source index
size_t *DBGManager_GetSrcNumLinesPtrFromIndex(size_t Index, bool Used)
{
	size_t *PtrNumLines = NULL;

	if ((DBGType & DBG_ELFDWARF))
	{
		PtrNumLines = DWARFManager_GetSrcNumLinesPtrFromIndex(Index, Used);
	}

	return	PtrNumLines;
}


// Get text source list pointers from source index
char **DBGManager_GetSrcListPtrFromIndex(size_t Index, bool Used)
{
	char **PtrSource = NULL;

	if ((DBGType & DBG_ELFDWARF))
	{
		PtrSource = DWARFManager_GetSrcListPtrFromIndex(Index, Used);
	}

	return	PtrSource;
}


// Get source language
size_t DBGManager_GetSrcLanguageFromIndex(size_t Index)
{
	size_t Language = 0;

	if ((DBGType & DBG_ELFDWARF))
	{
		Language = DWARFManager_GetSrcLanguageFromIndex(Index);
	}

	return	Language;
}

