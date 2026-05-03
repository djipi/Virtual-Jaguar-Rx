//
// baseremote.h: Abstract base class for Virtual Jaguar Rx remote implementations
//
// This is the abstract interface that all remote implementations must inherit from and implement.
// It defines the contract for remote communication, command handling, and integration with the emulator.
//
// Features:
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM  May/2026         Created this file
//

#ifndef __BASEREMOTE_H__
#define __BASEREMOTE_H__

#include <stdint.h>

//
struct baseinfosRemote
{
	uint32_t* cpu_d;      // Pointer to the CPU data registers
	uint32_t* cpu_a;      // Pointer to the CPU address registers
	uint16_t* cpu_sr;     // Pointer to the CPU status register
	uint32_t* cpu_pc;     // Pointer to the CPU program counter
	uint8_t* mem;         // Pointer to the memory buffer
	uint32_t mem_size;    // Size of the memory buffer in bytes
};

// Base class for all remotes
class baseRemote
{
public:
	virtual bool Init(uint32_t port, const baseinfosRemote* info) = 0;
	virtual bool Start(void) = 0;
	virtual void Close(void) = 0;
};

#endif
