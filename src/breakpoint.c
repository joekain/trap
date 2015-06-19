#include <trap.h>
#include "ptrace_util.h"
#include <stdio.h>
#include <stdint.h>

static trap_breakpoint_callback_t g_callback;
static uintptr_t g_original_breakpoint_word;

void trap_breakpoint_set_callback(trap_breakpoint_callback_t callback)
{
  g_callback = callback;
}

trap_breakpoint_t trap_inferior_set_breakpoint(trap_inferior_t inferior,
                                               char *location)
{
  const uintptr_t target_address = 0x000000000040079d;
  const uintptr_t int3_opcode = 0xCC;
  pid_t inferior_pid = inferior;
  uintptr_t modified_word;

  uintptr_t aligned_address = target_address & ~(0x7UL);
  uintptr_t target_offset = target_address - aligned_address;


  g_original_breakpoint_word = ptrace_util_peek_text(inferior_pid,
						     aligned_address);
  printf("Original word: 0x%016lx\n", g_original_breakpoint_word);
  modified_word = g_original_breakpoint_word;
  modified_word &= ~(0xFFUL << (target_offset * 8));
  modified_word |= int3_opcode << (target_offset * 8);
  printf("Original word: 0x%016lx\n", modified_word);
  ptrace_util_poke_text(inferior_pid, aligned_address, modified_word);

  return 0;
}

static void breakpoint_trigger_callback(trap_inferior_t inferior,
					trap_breakpoint_t handle)
{
  (*g_callback)(inferior, handle);
}

static trap_breakpoint_t breakpoint_resolve(trap_inferior_t inferior)
{
  return 0;
}

static void breakpoint_remove(trap_inferior_t inferior,
			      trap_breakpoint_t handle)
{
  unsigned long target_address = 0x000000000040079d;
  pid_t inferior_pid = inferior;

  ptrace_util_poke_text(inferior_pid, target_address, 
			g_original_breakpoint_word);
}

void breakpoint_handle(trap_inferior_t inferior)
{
  struct user_regs_struct regs;
  pid_t pid = inferior;

  trap_breakpoint_t bp = breakpoint_resolve(inferior);
  breakpoint_remove(inferior, bp);
  breakpoint_trigger_callback(inferior, bp);

  ptrace_util_get_regs(pid, &regs);
  regs.rip -= 1;
  ptrace_util_set_regs(pid, &regs);

  ptrace_util_continue(pid);
}
