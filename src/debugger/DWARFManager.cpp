//
// DWARFManager.cpp: DWARF format manager
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM  12/03/2016  Created this file
// JPM  12/03/2016  DWARF format support


#include	<stdlib.h>
#include	<stdio.h>
#include	<stdint.h>
#include	<string.h>
#include	<libdwarf.h>
#include	<dwarf.h>


// Source line internal structure
struct DMIStruct_LineSrc
{
	size_t Tag;
	size_t StartPC;
	size_t NumLineSrc;
	char *PtrLineSrc;
}S_DMIStruct_LineSrc;

// Base type internal structure
struct BaseTypeStruct
{
	size_t Tag;								// Type's Tag
	size_t Offset;							// Type's offset
	size_t TypeOffset;						// Type's offset on another type
	size_t ByteSize;						// Type's Byte Size
	size_t Encoding;						// Type's encoding
	char *PtrName;							// Type's name
}S_BaseTypeStruct;

// Variables internal structure
struct VariablesStruct
{
	size_t Addr;							// Variable memory address
	char *PtrName;							// Variable's name
	size_t TypeOffset;						// Offset pointing on the Variable's Type
	size_t TypeByteSize;					// Variable's Type byte size
	size_t TypeTag;							// Variable's Type Tag
	size_t TypeEncoding;					// Variable's Type encoding
	char *PtrTypeName;						// Variable's Type name
}S_VariablesStruct;

// Sub program internal structure
struct SubProgStruct
{
	size_t Tag;
	size_t NumLineSrc;
	size_t StartPC;
	size_t LowPC, HighPC;
	char *PtrLineSrc;
	char *PtrSubprogramName;
	size_t NbLinesSrc;
	DMIStruct_LineSrc *PtrLinesSrc;
}S_SubProgStruct;

// Compilation Unit internal structure
struct CUStruct
{
	size_t Tag;
	size_t LowPC, HighPC;
	char *PtrProducer;								// Pointer to the "Producer" information (compiler and compilation options used)
	char *PtrFullFilename;							// Pointer to full namefile (directory & filename)
	size_t SizeLoadSrc;								// Source code size
	char *PtrLoadSrc;								// Pointer to loaded source code
	size_t NbLinesLoadSrc;							// Lines source number
	char **PtrLinesLoadSrc;							// Pointer lists to each source line put in QT html/text conformity
	size_t NbSubProgs;								// Number of sub programs / routines
	SubProgStruct *PtrSubProgs;						// Pointer to the sub programs / routines information structure
	size_t NbTypes;
	BaseTypeStruct *PtrTypes;
	size_t NbVariables;								// Variables number
	VariablesStruct *PtrVariables;					// Pointer to the variables list information structure
}S_CUStruct;


// Dwarf management
uint32_t LibDwarf;
uint32_t NbCU;
Dwarf_Ptr errarg;
Dwarf_Error error;
Dwarf_Debug dbg;
CUStruct *PtrCU;


//
Dwarf_Handler DWARFManager_ErrorHandler(Dwarf_Ptr perrarg);
void DWARFManager_InitDMI(void);
void DWARFManager_CloseDMI(void);
bool DWARFManager_ElfClose(void);
char *DWARFManager_GetLineSrcFromNumLine(char *PtrSrcFile, size_t NumLine);


//
Dwarf_Handler DWARFManager_ErrorHandler(Dwarf_Ptr perrarg)
{
	return	0;
}


// Dwarf manager init
void DWARFManager_Init(void)
{
	LibDwarf = DW_DLV_NO_ENTRY;
}


// Dwarf manager Reset
bool DWARFManager_Reset(void)
{
	return DWARFManager_ElfClose();
}


// Dwarf manager Close
bool DWARFManager_Close(void)
{
	return(DWARFManager_Reset());
}


// Dwarf manager Elf init
int	DWARFManager_ElfInit(Elf *ElfPtr)
{
	if ((LibDwarf = dwarf_elf_init(ElfPtr, DW_DLC_READ, (Dwarf_Handler)DWARFManager_ErrorHandler, errarg, &dbg, &error)) == DW_DLV_OK)
	{
		DWARFManager_InitDMI();
	}

	return LibDwarf;
}


// Dwarf manager Elf close
bool DWARFManager_ElfClose(void)
{
	if (LibDwarf == DW_DLV_OK)
	{
		DWARFManager_CloseDMI();

		if (dwarf_finish(dbg, &error) == DW_DLV_OK)
		{
			LibDwarf = DW_DLV_NO_ENTRY;
			return	true;
		}
		else
		{
			return	false;
		}
	}
	else
	{
		return	true;
	}
}


// Dwarf manager Compilation Units close
void DWARFManager_CloseDMI(void)
{
	while (NbCU--)
	{
		free(PtrCU[NbCU].PtrFullFilename);
		free(PtrCU[NbCU].PtrLoadSrc);
		free(PtrCU[NbCU].PtrProducer);

		while (PtrCU[NbCU].NbLinesLoadSrc--)
		{
			free(PtrCU[NbCU].PtrLinesLoadSrc[PtrCU[NbCU].NbLinesLoadSrc]);
		}
		free(PtrCU[NbCU].PtrLinesLoadSrc);

		while (PtrCU[NbCU].NbSubProgs--)
		{
			free(PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].PtrLinesSrc);
			free(PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].PtrSubprogramName);
		}
		free(PtrCU[NbCU].PtrSubProgs);

		while (PtrCU[NbCU].NbTypes--)
		{
			free(PtrCU[NbCU].PtrTypes[PtrCU[NbCU].NbTypes].PtrName);
		}
		free(PtrCU[NbCU].PtrTypes);

		while (PtrCU[NbCU].NbVariables--)
		{
			free(PtrCU[NbCU].PtrVariables[PtrCU[NbCU].NbVariables].PtrName);
			free(PtrCU[NbCU].PtrVariables[PtrCU[NbCU].NbVariables].PtrTypeName);
		}
		free(PtrCU[NbCU].PtrVariables);
	}

	free(PtrCU);
}


// Dwarf manager Compilation Units initialisations
void DWARFManager_InitDMI(void)
{
	Dwarf_Unsigned	next_cu_header, return_uvalue;
	Dwarf_Error	error;
	Dwarf_Attribute	*atlist;
	Dwarf_Attribute	return_attr1;
	Dwarf_Half return_tagval, return_attr;
	Dwarf_Addr return_lowpc, return_highpc, return_lineaddr;
	Dwarf_Block *return_block;
	Dwarf_Signed atcnt, cnt;
	Dwarf_Die return_sib, return_die;
	Dwarf_Off return_offset;
	Dwarf_Line *linebuf;
	FILE *SrcFile;
	size_t i, j, k, TypeOffset;
	char *return_string;
	char *Ptr;
	char *SourceFilename = NULL;
	char *SourceFileDirectory = NULL;
	char *SourceFullFilename = NULL;

	// Initialisation for the Compilation Units table
	NbCU = 0;
	PtrCU = NULL;

	// loop on the available Compilation Unit
	while (dwarf_next_cu_header(dbg, NULL, NULL, NULL, NULL, &next_cu_header, &error) == DW_DLV_OK)
	{
		// Allocation of an additional Compilation Unit structure in the table
		if (Ptr = (char *)realloc(PtrCU, ((NbCU + 1) * sizeof(CUStruct))))
		{
			// Compilation Unit RAZ
			PtrCU = (CUStruct *)Ptr;
			memset(PtrCU + NbCU, 0, sizeof(CUStruct));

			// Get 1st Die from the Compilation Unit
			if (dwarf_siblingof(dbg, NULL, &return_sib, &error) == DW_DLV_OK)
			{
				// Get Die's Tag
				if ((dwarf_tag(return_sib, &return_tagval, &error) == DW_DLV_OK))
				{
					PtrCU[NbCU].Tag = return_tagval;

					// Die type detection
					switch (return_tagval)
					{
					case	DW_TAG_compile_unit:
						if (dwarf_attrlist(return_sib, &atlist, &atcnt, &error) == DW_DLV_OK)
						{
							for (Dwarf_Signed i = 0; i < atcnt; ++i)
							{
								if (dwarf_whatattr(atlist[i], &return_attr, &error) == DW_DLV_OK)
								{
									switch (return_attr)
									{
									case	DW_AT_low_pc:
										if (dwarf_lowpc(return_sib, &return_lowpc, &error) == DW_DLV_OK)
										{
											PtrCU[NbCU].LowPC = return_lowpc;
										}
										break;

									case	DW_AT_high_pc:
										if (dwarf_highpc(return_sib, &return_highpc, &error) == DW_DLV_OK)
										{
											PtrCU[NbCU].HighPC = return_highpc;
										}
										break;

									case	DW_AT_producer:
										if (dwarf_formstring(atlist[i], &return_string, &error) == DW_DLV_OK)
										{
											PtrCU[NbCU].PtrProducer = (char *)calloc(strlen(return_string) + 1, 1);
											strcpy(PtrCU[NbCU].PtrProducer, return_string);
											dwarf_dealloc(dbg, return_string, DW_DLA_STRING);
										}
										break;

									case	DW_AT_name:
										if (dwarf_formstring(atlist[i], &return_string, &error) == DW_DLV_OK)
										{
											SourceFilename = (char *)realloc(SourceFilename, strlen(return_string) + 1);
											strcpy(SourceFilename, return_string);
											dwarf_dealloc(dbg, return_string, DW_DLA_STRING);
										}
										break;

									case	DW_AT_comp_dir:
										if (dwarf_formstring(atlist[i], &return_string, &error) == DW_DLV_OK)
										{
											SourceFileDirectory = (char *)realloc(SourceFileDirectory, strlen(return_string) + 1);
											strcpy(SourceFileDirectory, return_string);
											dwarf_dealloc(dbg, return_string, DW_DLA_STRING);
										}
										break;

									default:
										break;
									}
								}
								dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
							}
							dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
						}

						Ptr = SourceFullFilename = (char *)realloc(SourceFullFilename, strlen(SourceFilename) + strlen(SourceFileDirectory) + 2);
						sprintf(SourceFullFilename, "%s\\%s", SourceFileDirectory, SourceFilename);
						while (*Ptr)
						{
							if (*Ptr == '/')
							{
								*Ptr = '\\';
							}
							Ptr++;
						}
						PtrCU[NbCU].PtrFullFilename = (char *)calloc(strlen(SourceFullFilename) + 1, 1);
						strcpy((char *)PtrCU[NbCU].PtrFullFilename, SourceFullFilename);

#ifndef __CYGWIN__
						if (!fopen_s(&SrcFile, SourceFullFilename, "rt"))
#else
						if (!(SrcFile = fopen(SourceFullFilename, "rt")))
#endif
						{
							if (!fseek(SrcFile, 0, SEEK_END))
							{
								if ((PtrCU[NbCU].SizeLoadSrc = ftell(SrcFile)) > 0)
								{
									if (PtrCU[NbCU].PtrLoadSrc = Ptr = (char *)calloc((PtrCU[NbCU].SizeLoadSrc + 1), 1))
									{
										rewind(SrcFile);
										if (PtrCU[NbCU].SizeLoadSrc < fread(Ptr, 1, PtrCU[NbCU].SizeLoadSrc, SrcFile))
										{
											free(PtrCU[NbCU].PtrLoadSrc);
											PtrCU[NbCU].PtrLoadSrc = NULL;
											PtrCU[NbCU].SizeLoadSrc = 0;
										}
										else
										{
											do
											{
												if (*Ptr == 0xa)
												{
													PtrCU[NbCU].NbLinesLoadSrc++;
													*Ptr = 0;
												}
											} while (*++Ptr);
										}
									}
								}
							}
							fclose(SrcFile);
						}
						break;

					default:
						break;
					}
				}

				// Get the source lines table located in the Compilation Unit
				if (dwarf_srclines(return_sib, &linebuf, &cnt, &error) == DW_DLV_OK)
				{
				}

				if (dwarf_child(return_sib, &return_die, &error) == DW_DLV_OK)
				{
					do
					{
						return_sib = return_die;
						if ((dwarf_tag(return_die, &return_tagval, &error) == DW_DLV_OK))
						{
							switch (return_tagval)
							{
							case	DW_TAG_lexical_block:
								break;

							case	DW_TAG_variable:
								if (dwarf_attrlist(return_die, &atlist, &atcnt, &error) == DW_DLV_OK)
								{
									PtrCU[NbCU].PtrVariables = (VariablesStruct *)realloc(PtrCU[NbCU].PtrVariables, ((PtrCU[NbCU].NbVariables + 1) * sizeof(VariablesStruct)));
									memset(PtrCU[NbCU].PtrVariables + PtrCU[NbCU].NbVariables, 0, sizeof(VariablesStruct));

									for (Dwarf_Signed i = 0; i < atcnt; ++i)
									{
										if (dwarf_whatattr(atlist[i], &return_attr, &error) == DW_DLV_OK)
										{
											if (dwarf_attr(return_die, return_attr, &return_attr1, &error) == DW_DLV_OK)
											{
												switch (return_attr)
												{
												case	DW_AT_location:
													if (dwarf_formblock(return_attr1, &return_block, &error) == DW_DLV_OK)
													{
														if (return_block->bl_len == 5)
														{
															PtrCU[NbCU].PtrVariables[PtrCU[NbCU].NbVariables].Addr = (*((unsigned char *)(return_block->bl_data) + 1) << 24) + (*((unsigned char *)(return_block->bl_data) + 2) << 16) + (*((unsigned char *)(return_block->bl_data) + 3) << 8) + (*((unsigned char *)(return_block->bl_data) + 4));
														}
														dwarf_dealloc(dbg, return_block, DW_DLA_BLOCK);
													}
													break;

												case	DW_AT_type:
													if (dwarf_global_formref(return_attr1, &return_offset, &error) == DW_DLV_OK)
													{
														PtrCU[NbCU].PtrVariables[PtrCU[NbCU].NbVariables].TypeOffset = return_offset;
													}
													break;

												case	DW_AT_name:
													if (dwarf_formstring(return_attr1, &return_string, &error) == DW_DLV_OK)
													{
														PtrCU[NbCU].PtrVariables[PtrCU[NbCU].NbVariables].PtrName = (char *)calloc(strlen(return_string) + 1, 1);
														strcpy(PtrCU[NbCU].PtrVariables[PtrCU[NbCU].NbVariables].PtrName, return_string);
														dwarf_dealloc(dbg, return_string, DW_DLA_STRING);
													}
													break;

												default:
													break;
												}
											}
										}

										dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
									}

									PtrCU[NbCU].NbVariables++;

									dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
								}
								break;

							case	DW_TAG_base_type:
							case	DW_TAG_typedef:
							case	DW_TAG_structure_type:
							case	DW_TAG_pointer_type:
							case	DW_TAG_const_type:
							case	DW_TAG_array_type:
							case	DW_TAG_subrange_type:
							case	DW_TAG_subroutine_type:
								if (dwarf_attrlist(return_die, &atlist, &atcnt, &error) == DW_DLV_OK)
								{
									PtrCU[NbCU].PtrTypes = (BaseTypeStruct *)realloc(PtrCU[NbCU].PtrTypes, ((PtrCU[NbCU].NbTypes + 1) * sizeof(BaseTypeStruct)));
									memset(PtrCU[NbCU].PtrTypes + PtrCU[NbCU].NbTypes, 0, sizeof(BaseTypeStruct));
									PtrCU[NbCU].PtrTypes[PtrCU[NbCU].NbTypes].Tag = return_tagval;

									if (dwarf_dieoffset(return_die, &return_offset, &error) == DW_DLV_OK)
									{
										PtrCU[NbCU].PtrTypes[PtrCU[NbCU].NbTypes].Offset = return_offset;
									}

									for (Dwarf_Signed i = 0; i < atcnt; ++i)
									{
										if (dwarf_whatattr(atlist[i], &return_attr, &error) == DW_DLV_OK)
										{
											if (dwarf_attr(return_die, return_attr, &return_attr1, &error) == DW_DLV_OK)
											{
												switch (return_attr)
												{
												case	DW_AT_type:
													if (dwarf_global_formref(return_attr1, &return_offset, &error) == DW_DLV_OK)
													{
														PtrCU[NbCU].PtrTypes[PtrCU[NbCU].NbTypes].TypeOffset = return_offset;
													}
													break;

												case	DW_AT_byte_size:
													if (dwarf_formudata(return_attr1, &return_uvalue, &error) == DW_DLV_OK)
													{
														PtrCU[NbCU].PtrTypes[PtrCU[NbCU].NbTypes].ByteSize = return_uvalue;
													}
													break;

												case	DW_AT_encoding:
													if (dwarf_formudata(return_attr1, &return_uvalue, &error) == DW_DLV_OK)
													{
														PtrCU[NbCU].PtrTypes[PtrCU[NbCU].NbTypes].Encoding = return_uvalue;
													}
													break;

												case	DW_AT_name:
													if (dwarf_formstring(return_attr1, &return_string, &error) == DW_DLV_OK)
													{
														PtrCU[NbCU].PtrTypes[PtrCU[NbCU].NbTypes].PtrName = (char *)calloc(strlen(return_string) + 1, 1);
														strcpy(PtrCU[NbCU].PtrTypes[PtrCU[NbCU].NbTypes].PtrName, return_string);
														dwarf_dealloc(dbg, return_string, DW_DLA_STRING);
													}
													break;

												default:
													break;
												}
											}
										}

										dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
									}

									PtrCU[NbCU].NbTypes++;

									dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
								}
								break;

							case	DW_TAG_subprogram:
								if (dwarf_attrlist(return_die, &atlist, &atcnt, &error) == DW_DLV_OK)
								{
									PtrCU[NbCU].PtrSubProgs = (SubProgStruct *)realloc(PtrCU[NbCU].PtrSubProgs, ((PtrCU[NbCU].NbSubProgs + 1) * sizeof(SubProgStruct)));
									memset((void *)(PtrCU[NbCU].PtrSubProgs + PtrCU[NbCU].NbSubProgs), 0, sizeof(SubProgStruct));
									PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].Tag = return_tagval;

									for (Dwarf_Signed i = 0; i < atcnt; ++i)
									{
										if (dwarf_whatattr(atlist[i], &return_attr, &error) == DW_DLV_OK)
										{
											if (dwarf_attr(return_die, return_attr, &return_attr1, &error) == DW_DLV_OK)
											{
												switch (return_attr)
												{
												case	DW_AT_low_pc:
													if (dwarf_lowpc(return_die, &return_lowpc, &error) == DW_DLV_OK)
													{
														PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].StartPC = return_lowpc;
														PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].LowPC = return_lowpc;
													}
													break;

												case	DW_AT_high_pc:
													if (dwarf_highpc(return_die, &return_highpc, &error) == DW_DLV_OK)
													{
														PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].HighPC = return_highpc;
													}
												break;

												case	DW_AT_decl_line:
													if (dwarf_formudata(return_attr1, &return_uvalue, &error) == DW_DLV_OK)
													{
														PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].NumLineSrc = return_uvalue;
													}
												break;

												case	DW_AT_name:
													if (dwarf_formstring(return_attr1, &return_string, &error) == DW_DLV_OK)
													{
														PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].PtrSubprogramName = (char *)calloc(strlen(return_string) + 1, 1);
														strcpy(PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].PtrSubprogramName, return_string);
														dwarf_dealloc(dbg, return_string, DW_DLA_STRING);
													}
													break;

												default:
													break;
												}
											}
										}
										dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
									}
									dwarf_dealloc(dbg, atlist, DW_DLA_LIST);

									for (i = 0; i < (size_t)cnt; ++i)
									{
										if (dwarf_lineaddr(linebuf[i], &return_lineaddr, &error) == DW_DLV_OK)
										{
											if (dwarf_lineno(linebuf[i], &return_uvalue, &error) == DW_DLV_OK)
											{
												if ((return_lineaddr >= return_lowpc) && (return_lineaddr <= return_highpc))
												{
													PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].PtrLinesSrc = (DMIStruct_LineSrc *)realloc(PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].PtrLinesSrc, (PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].NbLinesSrc + 1) * sizeof(DMIStruct_LineSrc));
													memset((void *)(PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].PtrLinesSrc + PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].NbLinesSrc), 0, sizeof(DMIStruct_LineSrc));
													PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].PtrLinesSrc[PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].NbLinesSrc].StartPC = return_lineaddr;
													PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].PtrLinesSrc[PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].NbLinesSrc].NumLineSrc = return_uvalue;
													PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs].NbLinesSrc++;
												}
											}
										}								
									}
												
									PtrCU[NbCU].NbSubProgs++;
								}
								break;

							default:
								break;
							}
						}
					}
					while (dwarf_siblingof(dbg, return_sib, &return_die, &error) == DW_DLV_OK);
				}

				// Release the memory used by the source lines
				for (i = 0; i < (size_t)cnt; ++i)
					dwarf_dealloc(dbg, linebuf[i], DW_DLA_LINE);
				dwarf_dealloc(dbg, linebuf, DW_DLA_LIST);
			}

			// Set the source code lines for QT html/text conformity
			if (PtrCU[NbCU].NbLinesLoadSrc)
			{
				if (PtrCU[NbCU].PtrLinesLoadSrc = (char **)calloc(PtrCU[NbCU].NbLinesLoadSrc, sizeof(char *)))
				{
					for (j = 0; j < PtrCU[NbCU].NbLinesLoadSrc; j++)
					{
						if (PtrCU[NbCU].PtrLinesLoadSrc[j] = (char *)calloc(10000, sizeof(char)))
						{
							if (Ptr = DWARFManager_GetLineSrcFromNumLine(PtrCU[NbCU].PtrLoadSrc, (j + 1)))
							{
								i = 0;

								while (*Ptr)
								{
									switch (*Ptr)
									{
									case 9:
										strcat(PtrCU[NbCU].PtrLinesLoadSrc[j], "&nbsp;");
										i += strlen("&nbsp;");
										break;

									case '<':
										strcat(PtrCU[NbCU].PtrLinesLoadSrc[j], "&lt;");
										i += strlen("&lt;");
										break;

									case '>':
										strcat(PtrCU[NbCU].PtrLinesLoadSrc[j], "&gt;");
										i += strlen("&gt;");
										break;
#if 0
									case '&':
										strcpy(PtrCU[NbCU].PtrLinesLoadSrc[j], "&amp;");
										i += strlen("&amp;");
										break;
#endif
#if 0
									case '"':
										strcpy(PtrCU[NbCU].PtrLinesLoadSrc[j], "&quot;");
										i += strlen("&quot;");
										break;
#endif
									default:
										PtrCU[NbCU].PtrLinesLoadSrc[j][i++] = *Ptr;
										break;
									}
									Ptr++;
								}
							}
							PtrCU[NbCU].PtrLinesLoadSrc[j] = (char *)realloc(PtrCU[NbCU].PtrLinesLoadSrc[j], i + 1);
						}
					}
				}
			}
			else
			{
				// Set each source lines pointer to NULL
				i = PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs - 1].PtrLinesSrc[PtrCU[NbCU].PtrSubProgs[PtrCU[NbCU].NbSubProgs - 1].NbLinesSrc - 1].NumLineSrc;
				if (PtrCU[NbCU].PtrLinesLoadSrc = (char **)calloc(i, sizeof(char *)))
				{
					for (j = 0; j < i; j++)
					{
						PtrCU[NbCU].PtrLinesLoadSrc[j] = NULL;
					}
				}
			}

			// Init lines source information based on each source code line numbers
			for (j = 0; j < PtrCU[NbCU].NbSubProgs; j++)
			{
				PtrCU[NbCU].PtrSubProgs[j].PtrLineSrc = PtrCU[NbCU].PtrLinesLoadSrc[PtrCU[NbCU].PtrSubProgs[j].NumLineSrc - 1];

				for (k = 0; k < PtrCU[NbCU].PtrSubProgs[j].NbLinesSrc; k++)
				{
					PtrCU[NbCU].PtrSubProgs[j].PtrLinesSrc[k].PtrLineSrc = PtrCU[NbCU].PtrLinesLoadSrc[PtrCU[NbCU].PtrSubProgs[j].PtrLinesSrc[k].NumLineSrc - 1];
				}
			}

			// Init variables information based on types information
			for (i = 0; i < PtrCU[NbCU].NbVariables; i++)
			{
				PtrCU[NbCU].PtrVariables[i].PtrTypeName = (char *)calloc(1000, 1);
				TypeOffset = PtrCU[NbCU].PtrVariables[i].TypeOffset;

				for (j = 0; j < PtrCU[NbCU].NbTypes; j++)
				{
					if (TypeOffset == PtrCU[NbCU].PtrTypes[j].Offset)
					{
						switch (PtrCU[NbCU].PtrTypes[j].Tag)
						{
						case DW_TAG_structure_type:
							PtrCU[NbCU].PtrVariables[i].TypeTag |= 0x1;
							if ((TypeOffset = PtrCU[NbCU].PtrTypes[j].TypeOffset))
							{
								j = -1;
							}
							else
							{
								if ((PtrCU[NbCU].PtrVariables[i].TypeTag & 0x2))
								{
									strcat(PtrCU[NbCU].PtrVariables[i].PtrTypeName, " *");
								}
							}
							break;

						case DW_TAG_pointer_type:
							PtrCU[NbCU].PtrVariables[i].TypeTag |= 0x2;
							PtrCU[NbCU].PtrVariables[i].TypeByteSize = PtrCU[NbCU].PtrTypes[j].ByteSize;
							PtrCU[NbCU].PtrVariables[i].TypeEncoding = 0x10;
							if (!(TypeOffset = PtrCU[NbCU].PtrTypes[j].TypeOffset))
							{
								strcat(PtrCU[NbCU].PtrVariables[i].PtrTypeName, "void *");
							}
							else
							{
								j = -1;
							}
							break;

						case DW_TAG_typedef:
							PtrCU[NbCU].PtrVariables[i].TypeTag |= 0x20;
							strcat(PtrCU[NbCU].PtrVariables[i].PtrTypeName, PtrCU[NbCU].PtrTypes[j].PtrName);
							if ((TypeOffset = PtrCU[NbCU].PtrTypes[j].TypeOffset))
							{
								j = -1;
							}
							break;

						case DW_TAG_subrange_type:
							PtrCU[NbCU].PtrVariables[i].TypeTag |= 0x4;
							break;

						case DW_TAG_array_type:
							PtrCU[NbCU].PtrVariables[i].TypeTag |= 0x8;
							if ((TypeOffset = PtrCU[NbCU].PtrTypes[j].TypeOffset))
							{
								j = -1;
							}
							break;

						case DW_TAG_const_type:
							PtrCU[NbCU].PtrVariables[i].TypeTag |= 0x10;
							strcat(PtrCU[NbCU].PtrVariables[i].PtrTypeName, "const ");
							if ((TypeOffset = PtrCU[NbCU].PtrTypes[j].TypeOffset))
							{
								j = -1;
							}
							break;

						case DW_TAG_base_type:
							strcat(PtrCU[NbCU].PtrVariables[i].PtrTypeName, PtrCU[NbCU].PtrTypes[j].PtrName);
							if ((PtrCU[NbCU].PtrVariables[i].TypeTag & 0x2))
							{
								strcat(PtrCU[NbCU].PtrVariables[i].PtrTypeName, " *");
							}
							else
							{
								PtrCU[NbCU].PtrVariables[i].TypeByteSize = PtrCU[NbCU].PtrTypes[j].ByteSize;
								PtrCU[NbCU].PtrVariables[i].TypeEncoding = PtrCU[NbCU].PtrTypes[j].Encoding;
							}
							if ((PtrCU[NbCU].PtrVariables[i].TypeTag & 0x8))
							{
								strcat(PtrCU[NbCU].PtrVariables[i].PtrTypeName, "[]");
							}
							break;

						default:
							break;
						}
					}
				}
			}

			++NbCU;
		}
	} 

	free(SourceFilename);
	free(SourceFileDirectory);
	free(SourceFullFilename);
}


// Get symbol name based from address
// Return NULL if no symbol name exists
char *DWARFManager_GetSymbolnameFromAdr(size_t Adr)
{
	size_t i, j;

	for (i = 0; i < NbCU; i++)
	{
		if ((Adr >= PtrCU[i].LowPC) && (Adr < PtrCU[i].HighPC))
		{
			for (j = 0; (j < PtrCU[i].NbSubProgs); j++)
			{
				if ((PtrCU[i].PtrSubProgs[j].StartPC == Adr))
				{
					return PtrCU[i].PtrSubProgs[j].PtrSubprogramName;
				}
			}
		}
	}

	return NULL;
}


// Get complete source filename based from address
// Return NULL if no source filename exists
// Return the existence status (true or false) in Error
char *DWARFManager_GetFullSourceFilenameFromAdr(size_t Adr, bool *Error)
{
	size_t i;

	for (i = 0; i < NbCU; i++)
	{
		if ((Adr >= PtrCU[i].LowPC) && (Adr < PtrCU[i].HighPC))
		{
			*Error = PtrCU[i].PtrLoadSrc ? true : false;
			return PtrCU[i].PtrFullFilename;
		}
	}

	return	NULL;
}


// Get text line source based on line number (starting by 1)
// Return NULL if no text line exists or if line number is 0
char *DWARFManager_GetLineSrcFromNumLine(char *PtrSrcFile, size_t NumLine)
{
	size_t i = 0;
	char *PtrLineSrc = NULL;

	if (PtrSrcFile)
	{
		while (i != NumLine)
		{
			PtrLineSrc = PtrSrcFile;
			while (*PtrSrcFile++);
			i++;
		}
	}

	return PtrLineSrc;
}


// Get Compilation Unit / External variables numbers
// Return variables number
size_t DWARFManager_GetNbExternalVariables(void)
{
	size_t NbVariables = 0, i;

	for (i = 0; i < NbCU; i++)
	{
		NbVariables += PtrCU[i].NbVariables;
	}

	return NbVariables;
}


// Get external variable type name based on his index (starting by 1)
// Return NULL if not found
// May return NULL if there is not type linked to the variable's index
char *DWARFManager_GetExternalVariableTypeName(size_t Index)
{
	size_t i;

	for (i = 0; i < NbCU; i++)
	{
		if (PtrCU[i].NbVariables)
		{
			if (Index <= PtrCU[i].NbVariables)
			{
				return PtrCU[i].PtrVariables[Index - 1].PtrTypeName;
			}
			else
			{
				Index -= PtrCU[i].NbVariables;
			}
		}
	}

	return NULL;
}


// Get external variable's type tag based on his index (starting by 1)
// Return 0 if not found
size_t DWARFManager_GetExternalVariableTypeTag(size_t Index)
{
	size_t i;

	for (i = 0; i < NbCU; i++)
	{
		if (PtrCU[i].NbVariables)
		{
			if (Index <= PtrCU[i].NbVariables)
			{
				return PtrCU[i].PtrVariables[Index - 1].TypeTag;
			}
			else
			{
				Index -= PtrCU[i].NbVariables;
			}
		}
	}

	return 0;
}


// Get external variable byte size based on his index (starting by 1)
// Return 0 if not found
size_t DWARFManager_GetExternalVariableTypeByteSize(size_t Index)
{
	size_t i;

	for (i = 0; i < NbCU; i++)
	{
		if (PtrCU[i].NbVariables)
		{
			if (Index <= PtrCU[i].NbVariables)
			{
				return PtrCU[i].PtrVariables[Index - 1].TypeByteSize;
			}
			else
			{
				Index -= PtrCU[i].NbVariables;
			}
		}
	}

	return 0;
}


// Get external variable encoding based on his index (starting by 1)
// Return 0 if not found
size_t DWARFManager_GetExternalVariableTypeEncoding(size_t Index)
{
	size_t i;

	for (i = 0; i < NbCU; i++)
	{
		if (PtrCU[i].NbVariables)
		{
			if (Index <= PtrCU[i].NbVariables)
			{
				return PtrCU[i].PtrVariables[Index - 1].TypeEncoding;
			}
			else
			{
				Index -= PtrCU[i].NbVariables;
			}
		}
	}

	return 0;
}


// Get external variable address based on his index (starting by 1)
// Return 0 if not found
size_t DWARFManager_GetExternalVariableAdr(size_t Index)
{
	size_t i;

	for (i = 0; i < NbCU; i++)
	{
		if (PtrCU[i].NbVariables)
		{
			if (Index <= PtrCU[i].NbVariables)
			{
				return PtrCU[i].PtrVariables[Index - 1].Addr;
			}
			else
			{
				Index -= PtrCU[i].NbVariables;
			}
		}
	}

	return 0;
}


// Get external variable memory address based on his name
// Return 0 if not found
// Note: Return the first occurence found
size_t DWARFManager_GetExternalVariableAdrFromName(char *VariableName)
{
	size_t i, j;

	for (i = 0; i < NbCU; i++)
	{
		if (PtrCU[i].NbVariables)
		{
			for (j = 0; j < PtrCU[i].NbVariables; j++)
			{
				if (!strcmp(PtrCU[i].PtrVariables[j].PtrName,VariableName))
				{
					return PtrCU[i].PtrVariables[j].Addr;
				}
			}
		}
	}

	return 0;
}


// Get external variable name based on his index (starting by 1)
// Return name's pointer text found
// Return NULL if not found
char *DWARFManager_GetExternalVariableName(size_t Index)
{
	size_t i;

	for (i = 0; i < NbCU; i++)
	{
		if (PtrCU[i].NbVariables)
		{
			if (Index <= PtrCU[i].NbVariables)
			{
				return PtrCU[i].PtrVariables[Index - 1].PtrName;
			}
			else
			{
				Index -= PtrCU[i].NbVariables;
			}
		}
	}

	return NULL;
}


// Get text line from source based on address and his tag
// Return NULL if no text line has been found
char *DWARFManager_GetLineSrcFromAdr(size_t Adr, size_t Tag)
{
	size_t i, j, k;

	for (i = 0; i < NbCU; i++)
	{
		if ((Adr >= PtrCU[i].LowPC) && (Adr < PtrCU[i].HighPC))
		{
			for (j = 0; j < PtrCU[i].NbSubProgs; j++)
			{
				if ((Adr >= PtrCU[i].PtrSubProgs[j].LowPC) && (Adr < PtrCU[i].PtrSubProgs[j].HighPC))
				{
					if ((PtrCU[i].PtrSubProgs[j].StartPC == Adr) && (!Tag || (Tag == DW_TAG_subprogram)))
					{
						return PtrCU[i].PtrSubProgs[j].PtrLineSrc;
					}
					else
					{
						for (k = 0; k < PtrCU[i].PtrSubProgs[j].NbLinesSrc; k++)
						{
							if ((PtrCU[i].PtrSubProgs[j].PtrLinesSrc[k].StartPC == Adr) && (!Tag || (PtrCU[i].PtrSubProgs[j].PtrLinesSrc[k].Tag == Tag)))
							{
								return PtrCU[i].PtrSubProgs[j].PtrLinesSrc[k].PtrLineSrc;
							}
						}
					}
				}
			}
		}
	}

	return	NULL;
}


// Get line number based on the address and the tag
// Return 0 if no line number has been found
size_t DWARFManager_GetNumLineFromAdr(size_t Adr, size_t Tag)
{
	size_t i, j, k;

	for (i = 0; i < NbCU; i++)
	{
		if ((Adr >= PtrCU[i].LowPC) && (Adr < PtrCU[i].HighPC))
		{
			for (j = 0; (j < PtrCU[i].NbSubProgs); j++)
			{
				if ((Adr >= PtrCU[i].PtrSubProgs[j].LowPC) && (Adr < PtrCU[i].PtrSubProgs[j].HighPC))
				{
					if ((PtrCU[i].PtrSubProgs[j].StartPC == Adr) && (!Tag || (Tag == DW_TAG_subprogram)))
					{
						return PtrCU[i].PtrSubProgs[j].NumLineSrc;
					}
					else
					{
						for (k = 0; (k < PtrCU[i].PtrSubProgs[j].NbLinesSrc); k++)
						{
							if ((PtrCU[i].PtrSubProgs[j].PtrLinesSrc[k].StartPC == Adr) && (!Tag || (PtrCU[i].PtrSubProgs[j].PtrLinesSrc[k].Tag == Tag)))
							{
								return PtrCU[i].PtrSubProgs[j].PtrLinesSrc[k].NumLineSrc;
							}
						}
					}
#if 0
					if (!Tag || (Tag == DW_TAG_subprogram))
					{
						return PtrCU[i].PtrSubProgs[j].NumLineSrc;
					}
#endif
				}
			}
		}
	}

	return	0;
}


// Get text line from source based on address and num line (starting by 1)
// Return NULL if no text line has been found
char *DWARFManager_GetLineSrcFromAdrNumLine(size_t Adr, size_t NumLine)
{
	size_t i, j, k;

	for (i = 0; i < NbCU; i++)
	{
		if ((Adr >= PtrCU[i].LowPC) && (Adr < PtrCU[i].HighPC))
		{
			for (j = 0; j < PtrCU[i].NbSubProgs; j++)
			{
				if ((Adr >= PtrCU[i].PtrSubProgs[j].LowPC) && (Adr < PtrCU[i].PtrSubProgs[j].HighPC))
				{
					if (PtrCU[i].PtrSubProgs[j].NumLineSrc == NumLine)
					{
						return PtrCU[i].PtrSubProgs[j].PtrLineSrc;
					}
					else
					{
						for (k = 0; k < PtrCU[i].PtrSubProgs[j].NbLinesSrc; k++)
						{
							if (PtrCU[i].PtrSubProgs[j].PtrLinesSrc[k].NumLineSrc == NumLine)
							{
								return PtrCU[i].PtrSubProgs[j].PtrLinesSrc[k].PtrLineSrc;
							}
						}
					}
				}
			}
		}
	}

	return NULL;
}


// Get text line from source based on address and num line (starting by 1)
// Return NULL if no text line has been found
char *DWARFManager_GetLineSrcFromNumLineBaseAdr(size_t Adr, size_t NumLine)
{
	size_t i;

	for (i = 0; i < NbCU; i++)
	{
		if ((Adr >= PtrCU[i].LowPC) && (Adr < PtrCU[i].HighPC))
		{
			return PtrCU[i].PtrLinesLoadSrc[NumLine - 1];
		}
	}

	return NULL;
}

