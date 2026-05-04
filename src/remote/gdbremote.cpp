//
// gdbremote.cpp - GDB remote stub integration for Virtual Jaguar Rx
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM  May/2026         Created this file
//

#include <thread> 
#include "gdbremote.h"


// Constructor initializes the GDB stub configuration structure with default values
GDBRemote::GDBRemote(void) :
gdbstub_cpu{ nullptr, nullptr, nullptr, nullptr },
gdbstub_cfg{ nullptr, nullptr, 0, nullptr, nullptr, nullptr, nullptr }
{
	// initialization for the GDB stub configuration structure
	gdbstub_cfg.cpu = &gdbstub_cpu;
	// redirect the log output of the GDB stub to the emulator's logging system
	gdbstub_set_log_callback(nullptr);
}


//
bool GDBRemote::Init(uint32_t port, const baseinfosRemote* info)
{
	// fill the GDB stub configuration structure with the CPU information
	gdbstub_cfg.cpu->a = info->cpu_a;
	gdbstub_cfg.cpu->d = info->cpu_d;
	gdbstub_cfg.cpu->sr = info->cpu_sr;
	gdbstub_cfg.cpu->pc = info->cpu_pc;
	// fill the GDB stub configuration for the memory information
	gdbstub_cfg.mem = info->mem;
	gdbstub_cfg.mem_size = info->mem_size;
	// fill the GDB stub configuration structure with the callback functions
	gdbstub_cfg.step_cb = nullptr;
	gdbstub_cfg.run_cb = nullptr;
	gdbstub_cfg.add_bp_cb = &m68k_brk_add_addr;
	gdbstub_cfg.del_bp_cb = &m68k_brk_del_addr;

	// initialize the GDB stub with the specified port and configuration
	return gdbstub_init(port, &gdbstub_cfg);
}


//
bool GDBRemote::Start(void)
{
	std::thread gdb_thread([]() { gdbstub_start(); });
	gdb_thread.detach();
	return true;
}


//
void GDBRemote::Close(void)
{
	gdbstub_close();
}


//
GDBRemote::~GDBRemote()
{
}
