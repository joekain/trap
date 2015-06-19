#include <trap.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <stdio.h>

static const void *ignored_ptr;
static const void *no_continue_signal = 0;

static trap_breakpoint_callback_t g_callback;
static unsigned char g_original_breakpoint_byte;

void trap_breakpoint_set_callback(trap_breakpoint_callback_t callback)
{
  g_callback = callback;
}

trap_breakpoint_t trap_inferior_set_breakpoint(trap_inferior_t inferior,
                                               char *location)
{
  const void *target_address = (void *)0x000000000040079d;
  const void *int3_opcode = (void *)0xCC;
  pid_t inferior_pid = inferior;

  ptrace(PTRACE_PEEKTEXT, inferior_pid, target_address,
	 &g_original_breakpoint_byte);
  ptrace(PTRACE_POKETEXT, inferior_pid, target_address, int3_opcode);

  return 0;
}

void breakpoint_trigger_callback(trap_inferior_t inferior,
				 trap_breakpoint_t handle)
{
  (*g_callback)(inferior, handle);
}

void breakpoint_resolve(trap_inferior_t inferior)
{
  return 0;
}

void breakpoint_remove(trap_inferior_t inferior, trap_breakpoint_t handle)
{
  const void *target_address = (void *)0x000000000040079d;
  const void *int3_opcode = (void *)0xCC;
  pid_t inferior_pid = inferior;

  ptrace(PTRACE_POKETEXT, inferior_pid, target_address,
	 g_original_breakpoint_byte);
}
