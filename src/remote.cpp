//
// remote.cpp - Remote information
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM  Apr./2026        Created this file
//

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "remote.h"
#include "remote/baseremote.h"
#include "remote/dummyremote.h"
#include "remote/gdbremote.h"
#include "m68000/cpudefs.h"
#include "memory.h"


// Remote type index
typedef enum { GDBREMOTE_IDX } RemoteIdx_t;
#define COUNT_REMOTES	1


// Specific to all remotes
baseRemote* BaseRemotes[COUNT_REMOTES] = { nullptr };
baseinfosRemote BaseInfosRemote = { regs.regs, &regs.regs[8], &regs.sr, &regs.pc, jagMemSpace, sizeof(jagMemSpace)};


// Remotes initialization
bool Remote_Init(uint32_t type, uint32_t port)
{
	bool ret;

	// GDB Remote setup
#ifdef GDBSTUB_ENABLE
	(type & GDBREMOTE) ? BaseRemotes[GDBREMOTE_IDX] = new GDBRemote() : BaseRemotes[GDBREMOTE_IDX] = new DummyRemote();
#else
	baseReremotes[GDBREMOTE_IDX] = new DummyRemote();
#endif

	// initialize all remotes
	for (size_t i = 0; i < COUNT_REMOTES; i++)
	{
		ret = BaseRemotes[i] ? BaseRemotes[i]->Init(port, &BaseInfosRemote) : false;
	}

	return ret;
}


// Remotes starting
bool Remote_Start(void)
{
	bool ret = true;

	// start all remotes
	for (size_t i = 0; i < COUNT_REMOTES; i++)
	{
		ret &= BaseRemotes[i] ? BaseRemotes[i]->Start() : false;
	}

	return ret;
}


// Remotes closing
void Remote_Close(void)
{
	// close and delete all remotes
	for (size_t i = 0; i < COUNT_REMOTES; i++)
	{
		BaseRemotes[i]->Close();
		delete BaseRemotes[i];
		BaseRemotes[i] = nullptr;
	}
}
