//
// gdbremote.h: GDB remote stub integration for Virtual Jaguar Rx
//
// This class provides an implementation of the baseRemote interface using the GDB remote protocol
//
// Features:
//
// Note: This header is only active when GDBSTUB_ENABLE is defined.
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM  May/2026         Created this file
//

#if !defined(__GDBREMOTE_H__) && defined(GDBSTUB_ENABLE)
#define __GDBREMOTE_H__

#include <stdint.h>
#include "baseRemote.h"

class GDBRemote : public baseRemote
{
public:
	GDBRemote(void);
	~GDBRemote(void);
	bool Init(uint32_t port, const baseinfosRemote* info) override;
	bool Start(void) override;
	void Close(void) override;
};

#endif
