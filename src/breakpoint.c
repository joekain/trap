#include <trap.h>
#include "ptrace_util.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

struct breakpoint_t {
  uintptr_t target_address;
  uintptr_t aligned_address;
  uintptr_t original_breakpoint_word;
};
typedef struct breakpoint_t breakpoint_t;

breakpoint_t g_breakpoint;

static trap_breakpoint_callback_t g_callback;

void trap_breakpoint_set_callback(trap_breakpoint_callback_t callback)
{
  g_callback = callback;
}

trap_breakpoint_t trap_inferior_set_breakpoint(trap_inferior_t inferior,
                                               char *location)
{
  const uintptr_t target_address = (uintptr_t)location;
  const uintptr_t int3_opcode = 0xCC;
  pid_t inferior_pid = inferior;
  uintptr_t modified_word;

  uintptr_t aligned_address = target_address & ~(0x7UL);
  uintptr_t target_offset = target_address - aligned_address;

  breakpoint_t *bp = &g_breakpoint;

  bp->original_breakpoint_word = ptrace_util_peek_text(inferior_pid,
						       aligned_address);
  bp->target_address = target_address;
  bp->aligned_address = aligned_address;

  modified_word = bp->original_breakpoint_word;
  modified_word &= ~(0xFFUL << (target_offset * 8));
  modified_word |= int3_opcode << (target_offset * 8);
  ptrace_util_poke_text(inferior_pid, aligned_address, modified_word);

  return bp;
}

static void breakpoint_trigger_callback(trap_inferior_t inferior,
					trap_breakpoint_t handle)
{
  (*g_callback)(inferior, handle);
}

static trap_breakpoint_t breakpoint_resolve(trap_inferior_t inferior)
{
  return &g_breakpoint;
}

static void breakpoint_remove(trap_inferior_t inferior,
			      trap_breakpoint_t handle)
{
  breakpoint_t *bp = (breakpoint_t *)handle;
  pid_t inferior_pid = inferior;

  ptrace_util_poke_text(inferior_pid, bp->aligned_address, 
			bp->original_breakpoint_word);
}

void breakpoint_handle(trap_inferior_t inferior)
{
  struct user_regs_struct regs;
  pid_t pid = inferior;

  trap_breakpoint_t bp = breakpoint_resolve(inferior);
  breakpoint_remove(inferior, bp);
  breakpoint_trigger_callback(inferior, bp);

  ptrace_util_get_regs(pid, &regs);
  ptrace_util_set_instruction_pointer(pid, regs.rip - 1);

  ptrace_util_continue(pid);
}
