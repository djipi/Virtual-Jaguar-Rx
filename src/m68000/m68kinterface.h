//
// C interface to the UAE core
//
// by James Hammons
// (C) 2011 Underground Software
//
// Modified by Jean-Paul Mari
//
// Most of these functions are in place to help make it easy to replace the
// Musashi core with my bastardized UAE one. :-)
//

#ifndef __M68KINTERFACE_H__
#define __M68KINTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Registers used by m68k_get_reg() and m68k_set_reg() */
typedef enum
{
	/* Real registers */
	M68K_REG_D0,		/* Data registers */
	M68K_REG_D1,
	M68K_REG_D2,
	M68K_REG_D3,
	M68K_REG_D4,
	M68K_REG_D5,
	M68K_REG_D6,
	M68K_REG_D7,
	M68K_REG_A0,		/* Address registers */
	M68K_REG_A1,
	M68K_REG_A2,
	M68K_REG_A3,
	M68K_REG_A4,
	M68K_REG_A5,
	M68K_REG_A6,
	M68K_REG_A7,
	M68K_REG_PC,		/* Program Counter */
	M68K_REG_SR,		/* Status Register */
	M68K_REG_SP,		/* The current Stack Pointer (located in A7) */
	M68K_REG_USP,		/* User Stack Pointer */

	/* Assumed registers */
	/* These are cheat registers which emulate the 1-longword prefetch
	 * present in the 68000 and 68010.
	 */ 
	M68K_REG_PREF_ADDR,	/* Last prefetch address */
	M68K_REG_PREF_DATA,	/* Last prefetch data */

	/* Convenience registers */
	M68K_REG_PPC,		/* Previous value in the program counter */
	M68K_REG_IR,		/* Instruction register */
} m68k_register_t;

/* Special interrupt acknowledge values.
 * Use these as special returns from the interrupt acknowledge callback
 * (specified later in this header).
 */

/* Causes an interrupt autovector (0x18 + interrupt level) to be taken.
 * This happens in a real 68K if VPA or AVEC is asserted during an interrupt
 * acknowledge cycle instead of DTACK.
 */
#define M68K_INT_ACK_AUTOVECTOR    0xFFFFFFFF

/* Causes the spurious interrupt vector (0x18) to be taken
 * This happens in a real 68K if BERR is asserted during the interrupt
 * acknowledge cycle (i.e. no devices responded to the acknowledge).
 */
#define M68K_INT_ACK_SPURIOUS      0xFFFFFFFE

extern void m68k_set_cpu_type(unsigned int);
extern void m68k_pulse_reset(void);
extern int m68k_execute(int num_cycles);
extern void m68k_set_irq(unsigned int int_level);
extern size_t m68k_dump(FILE *fp);
extern size_t m68k_load(FILE *fp);


// Functions that MUST be implemented by the user:

// Read from anywhere
extern unsigned int m68k_read_memory_8(unsigned int address);
extern unsigned int m68k_read_memory_16(unsigned int address);
extern unsigned int m68k_read_memory_32(unsigned int address);

// Write to anywhere
extern void m68k_write_memory_8(unsigned int address, unsigned int value);
extern void m68k_write_memory_16(unsigned int address, unsigned int value);
extern void m68k_write_memory_32(unsigned int address, unsigned int value);

extern int irq_ack_handler(int);

// Convenience functions

// Uncomment this to have the emulated CPU call a hook function after every instruction
// NB: This must be implemented by the user!
#define M68K_HOOK_FUNCTION
#ifdef M68K_HOOK_FUNCTION
extern void M68KInstructionHook(void);
#endif


extern int M68KGetCurrentOpcodeFamily(void);


// Functions to allow debugging
extern int M68KDebugHalt(void);
extern void M68KDebugResume(void);
extern int M68KDebugHaltStatus(void);

/* Peek at the internals of a CPU context.  This can either be a context
 * retrieved using m68k_get_context() or the currently running context.
 * If context is NULL, the currently running CPU context will be used.
 */
extern unsigned int m68k_get_reg(void * context, m68k_register_t reg);

/* Poke values into the internals of the currently running CPU context */
extern void m68k_set_reg(m68k_register_t reg, unsigned int value);

// Dummy functions, for now...

/* Check if an instruction is valid for the specified CPU type */
extern unsigned int m68k_is_valid_instruction(unsigned int instruction, unsigned int cpu_type);

/* Disassemble 1 instruction using the epecified CPU type at pc.  Stores
 * disassembly in str_buff and returns the size of the instruction in bytes.
 */
extern unsigned int m68k_disassemble(char * str_buff, unsigned int pc, unsigned int cpu_type, unsigned int OpCodes);

/* These functions let you read/write/modify the number of cycles left to run
 * while m68k_execute() is running.
 * These are useful if the 68k accesses a memory-mapped port on another device
 * that requires immediate processing by another CPU.
 */
//int m68k_cycles_run(void);              // Number of cycles run so far
//int m68k_cycles_remaining(void);        // Number of cycles left
extern void m68k_modify_timeslice(int cycles); // Modify cycles left
extern void m68k_end_timeslice(void);          // End timeslice now

// Breakpoints functions
extern void m68k_brk_init(void);
extern void m68k_brk_hitcounts_reset(void);
extern unsigned int m68k_brk_add(void *PtrInfo);
extern void m68k_brk_del(unsigned int NumBrk);
extern void m68k_brk_disable(void);
extern void m68k_brk_reset(void);
extern void m68k_brk_close(void);
extern unsigned int m68k_brk_check(unsigned int adr);

#ifdef __cplusplus
}
#endif

#endif	// __M68KINTERFACE_H__
