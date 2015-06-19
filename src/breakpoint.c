#include <trap.h>
#include <sys/ptrace.h>
#include <unistd.h>

static const void *ignored_ptr;
static const void *no_continue_signal = 0;

static trap_breakpoint_callback_t g_callback;

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

  ptrace(PTRACE_POKETEXT, inferior_pid, int3_opcode, target_address);

  return 0;
}
