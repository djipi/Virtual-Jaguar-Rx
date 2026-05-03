//
// remote.h: Virtual Jaguar Rx remote subsystem interface
//
// This is the main remote interface for Virtual Jaguar Rx, providing an unified API for remote capabilities.
//
// Features:
// - Multiple remote back-end support
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM  Apr./2026        Created this file
//

#ifndef __REMOTE_H__
#define __REMOTE_H__

#include <stdint.h>

// Remote types list (must be binary based)
typedef enum {	NOREMOTE = 0x0, GDBREMOTE = 0x1	} remoteType_t;

// Remote generic functions
extern bool Remote_Init(uint32_t type, uint32_t port);
extern bool Remote_Start(void);
extern void Remote_Close(void);

#endif
