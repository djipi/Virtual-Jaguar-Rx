// dummyremote.h: Null object implementation of the remote interface
//
// This class provides a no-op (do-nothing) implementation of the baseRemote interface, following the Null Object design pattern.
// It allows the remote subsystem to be disabled at runtime without requiring conditional checks throughout the codebase.
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM  May/2026         Created this file
//

#ifndef __DUMMYREMOTE_H__
#define __DUMMYREMOTE_H__

#include <stdint.h>
#include "baseremote.h"

class DummyRemote: public baseRemote
{
public:
	bool Init(uint32_t port, const baseinfosRemote* info) override { return true; }
    bool Start(void) override { return false; }
	void Close(void) override {}
};

#endif
