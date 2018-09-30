//
// state.cpp: VJ machine state save/load support
//
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/16/2010  Created this log ;-)
// JPM  09/26/2018  Added state Save/Load functionalities
//

// STILL TO DO:
// Checksum error could be useful
// Check missing states
// To allow savestate reading whatever the platform
// 


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "jaguar.h"
#include "m68000/m68kinterface.h"
#include "gpu.h"
#include "dsp.h"
#include "op.h"
#include "tom.h"
#include "blitter.h"
#include "dac.h"


// Savestate file signature
#ifdef _WIN32
#define SST_SIGNATURE	"JAGSST_WIN32"
#else
#define SST_SIGNATURE	"JAGSST_?"
#endif
// Debug
//#define DEBUG_SST	"test.sst"				// Force savestate filename


// Savestate data structure
typedef struct _SST
{
	unsigned char Signature[256];		// File signature
	unsigned char SSTM68K[200];			// M68000 information
	unsigned char SSTGPU[4700];			// GPU information
	unsigned char SSTDSP[9000];			// DSP information
	unsigned char SSTOP[33000];			// OP information
	unsigned char SSTTom[17000];		// Tom information
	unsigned char SSTBlitter[1000];		// Blitter information
	unsigned char SSTDac[1000];			// Dac information
	uint8_t FullJagMemory[0xF20000];	// Full Jaguar memory space
}SST;


// Create the savestate
bool SaveState(unsigned char *FileName)
{
	FILE *File;
	SST *Ptr;
	bool Success = false;
#ifdef DEBUG_SST
	FileName = (unsigned char *)DEBUG_SST;
#endif

	// Check filename
	if (FileName)
	{
		// Open file for writing
		if (File = fopen((const char *)FileName, "wb"))
		{
			// Allocate memory for the save state
			if ((Ptr = (SST *)calloc(1, sizeof(SST))))
			{
				// Set signature
				sprintf((char *)Ptr->Signature, "[%s_%s]", SST_SIGNATURE, __DATE__);
				// Set the M68000 information
				if (m68k_write_savestate(Ptr->SSTM68K) < sizeof((_SST *)0)->SSTM68K)
				{
					// Set the GPU information
					if (GPUWriteSavestate(Ptr->SSTGPU) < sizeof((_SST *)0)->SSTGPU)
					{
						// Set the DSP information
						if (DSPWriteSavestate(Ptr->SSTDSP) < sizeof((_SST *)0)->SSTDSP)
						{
							// Set the OP information
							if (OPWriteSavestate(Ptr->SSTOP) < sizeof((_SST *)0)->SSTOP)
							{
								// Set the Tom information
								if (TomWriteSavestate(Ptr->SSTTom) < sizeof((_SST *)0)->SSTTom)
								{
									// Set the Blitter information
									if (BlitterWriteSavestate(Ptr->SSTBlitter) < sizeof((_SST *)0)->SSTBlitter)
									{
										// Set the Dac information
										if (DacWriteSavestate(Ptr->SSTDac) < sizeof((_SST *)0)->SSTDac)
										{
											// Set the entire Jaguar memory
											memcpy((void *)Ptr->FullJagMemory, (void *)jagMemSpace, 0xF20000);
											// Write save state data
											Success = (fwrite(Ptr, (size_t)sizeof(SST), 1, File) == 1);
										}
									}
								}
							}
						}
					}
				}

				free(Ptr);
			}

			fclose(File);
		}
	}

	return Success;
}


// Load savestate and distribute data
bool LoadState(unsigned char *FileName)
{
	FILE *File;
	SST *Ptr;
	unsigned char Buffer[32];
	bool Success = false;
#ifdef DEBUG_SST
	FileName = (unsigned char *)DEBUG_SST;
#endif

	// Check filename
	if (FileName)
	{
		// Open file for reading
		if (File = fopen((const char *)FileName, "rb"))
		{	
			// Allocate memory for the save state
			if ((Ptr = (SST *)calloc(1, sizeof(SST))))
			{
				// Read the save state data
				if (Success = (fread(Ptr, (size_t)sizeof(SST), 1, File) == 1))
				{
					// Check signature
					sprintf((char *)Buffer, "[%s_%s]", SST_SIGNATURE, __DATE__);
					if (!strncmp((const char *)Ptr->Signature, (const char *)Buffer, sizeof(Buffer)))
					{
						// Get the entire Jaguar memory
						memcpy((void *)jagMemSpace, (void *)Ptr->FullJagMemory, 0xF20000);
						// Get the M68000 information
						m68k_read_savestate(Ptr->SSTM68K);
						// Get the GPU information
						GPUReadSavestate(Ptr->SSTGPU);
						// Get the DSP information
						DSPReadSavestate(Ptr->SSTDSP);
						// Get the OP information
						OPReadSavestate(Ptr->SSTOP);
						// Get the Tom information
						TomReadSavestate(Ptr->SSTTom);
						// Get the Blitter information
						BlitterReadSavestate(Ptr->SSTBlitter);
						// Get the Dac information
						DacReadSavestate(Ptr->SSTDac);

						Success = true;
					}
				}

				free(Ptr);
			}
		}

		fclose(File);
	}

	return Success;
}

