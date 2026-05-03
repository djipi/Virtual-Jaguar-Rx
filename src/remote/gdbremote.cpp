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
#include "gdbstub.h"


//
GDBRemote::GDBRemote(void)
{
	gdbstub_set_log_callback(NULL);
}


//
bool GDBRemote::Init(uint32_t port, const baseinfosRemote* info)
{
	// fill the GDB stub configuration structure with the CPU and memory information from the baseinfosRemote struct
	gdbstub_config_t gdbstub_cfg;
	gdbstub_cfg.cpu->a = info->cpu_a;
	gdbstub_cfg.cpu->d = info->cpu_d;
	gdbstub_cfg.cpu->sr = info->cpu_sr;
	gdbstub_cfg.cpu->pc = info->cpu_pc;
	gdbstub_cfg.mem = info->mem;
	gdbstub_cfg.mem_size = info->mem_size;

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
