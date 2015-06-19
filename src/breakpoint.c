#include <trap.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/user.h>

static trap_breakpoint_callback_t g_callback;
static uintptr_t g_original_breakpoint_word;
static const void *ignored_ptr;
static const void *no_continue_signal = 0;

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


  g_original_breakpoint_word = ptrace(PTRACE_PEEKTEXT, inferior_pid,
				      aligned_address, 0);
  printf("Original word: 0x%016lx\n", g_original_breakpoint_word);
  modified_word = g_original_breakpoint_word;
  modified_word &= ~(0xFFUL << (target_offset * 8));
  modified_word |= int3_opcode << (target_offset * 8);
  printf("Original word: 0x%016lx\n", modified_word);
  ptrace(PTRACE_POKETEXT, inferior_pid, aligned_address, modified_word);

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
  const void *target_address = (void *)0x000000000040079d;
  pid_t inferior_pid = inferior;

  ptrace(PTRACE_POKETEXT, inferior_pid, target_address,
	 g_original_breakpoint_word);
}

void breakpoint_handle(trap_inferior_t inferior)
{
  struct user_regs_struct regs;
  pid_t pid = inferior;

  trap_breakpoint_t bp = breakpoint_resolve(inferior);
  breakpoint_remove(inferior, bp);
  breakpoint_trigger_callback(inferior, bp);

  ptrace(PTRACE_GETREGS, pid, ignored_ptr, &regs);
  regs.rip -= 1;
  ptrace(PTRACE_SETREGS, pid, ignored_ptr, &regs);

  ptrace(PTRACE_CONT, pid, ignored_ptr, no_continue_signal);
}
