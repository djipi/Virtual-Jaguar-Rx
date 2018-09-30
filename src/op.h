//
// OBJECTP.H: Object Processor header file
//

#ifndef __OBJECTP_H__
#define __OBJECTP_H__

// General functions
extern void OPInit(void);
extern void OPReset(void);
extern void OPDone(void);

// Savestate functions
extern uint32_t OPReadSavestate(unsigned char *ptrsst);
extern uint32_t OPWriteSavestate(unsigned char *ptrsst);

// OP memory access
extern uint64_t OPLoadPhrase(uint32_t offset);
extern void OPProcessList(int scanline, bool render);
extern uint32_t OPGetListPointer(void);

#define OPFLAG_RELEASE		8					// Bus release bit
#define OPFLAG_TRANS		4					// Transparency bit
#define OPFLAG_RMW			2					// Read-Modify-Write bit
#define OPFLAG_REFLECT		1					// Horizontal mirror bit

// Variables
extern uint8_t objectp_running;

#endif	// __OBJECTP_H__
