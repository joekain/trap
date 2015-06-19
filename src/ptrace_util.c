#include "ptrace_util.h"
#include <sys/ptrace.h>

/* Used for ignored arguments */
static const pid_t ignored_pid;
static const void *ignored_addr;
static const void *ignored_data;

void ptrace_util_traceme()
{
  ptrace(PTRACE_TRACEME, ignored_pid, ignored_addr, ignored_data);
}

void ptrace_util_continue(pid_t pid)
{
  static const void *no_continue_signal = 0;
  ptrace(PTRACE_CONT, pid, ignored_addr, no_continue_signal);
}

long ptrace_util_peek_text(pid_t pid, unsigned long target_address)
{
  return ptrace(PTRACE_PEEKTEXT, pid, (void *)target_address, ignored_data);
}

void ptrace_util_poke_text(pid_t pid, unsigned long target_address,
                           unsigned long data)
{
  ptrace(PTRACE_POKETEXT, pid, (void *)target_address, (void *)data);
}

void ptrace_util_get_regs(pid_t pid, struct user_regs_struct *regs)
{
  ptrace(PTRACE_GETREGS, pid, ignored_addr, regs);
}

void ptrace_util_set_regs(pid_t pid, struct user_regs_struct *regs)
{
  ptrace(PTRACE_SETREGS, pid, ignored_addr, &regs);
}
