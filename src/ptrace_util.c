#include "ptrace_util.h"
#include <sys/ptrace.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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
  int result = ptrace(PTRACE_CONT, pid, ignored_addr, no_continue_signal);
  if (result != 0) {
    perror("PTRACE_CONT: ");
    abort();
  }
}

long ptrace_util_peek_text(pid_t pid, unsigned long target_address)
{
  return ptrace(PTRACE_PEEKTEXT, pid, (void *)target_address, ignored_data);
}

void ptrace_util_poke_text(pid_t pid, unsigned long target_address,
                           unsigned long data)
{
  int result = ptrace(PTRACE_POKETEXT, pid, (void *)target_address, (void *)data);
  if (result != 0) {
    perror("PTRACE_POKETEXT: ");
    abort();
  }
}

void ptrace_util_get_regs(pid_t pid, struct user_regs_struct *regs)
{
  int result = ptrace(PTRACE_GETREGS, pid, ignored_addr, regs);
  if (result != 0) {
    perror("PTRACE_GETREGS: ");
    abort();
  }
}

void ptrace_util_set_regs(pid_t pid, struct user_regs_struct *regs)
{
  int result = ptrace(PTRACE_SETREGS, pid, ignored_addr, &regs);
  if (result != 0) {
    perror("PTRACE_SETREGS: ");
    abort();
  }
}

uintptr_t ptrace_util_get_instruction_pointer(pid_t pid)
{
  uintptr_t offset = offsetof(struct user, regs.rip);
  return ptrace(PTRACE_PEEKUSER, pid, offset, ignored_data);
}


void ptrace_util_set_instruction_pointer(pid_t pid, uintptr_t ip)
{
  uintptr_t offset = offsetof(struct user, regs.rip);
  int result = ptrace(PTRACE_POKEUSER, pid, offset, ip);
  if (result != 0) {
    perror("ptrace_util_set_instruction_pointer: ");
    abort();
  }
}

void ptrace_util_single_step(pid_t pid)
{
  static const void *no_step_signal = 0;
  int result = ptrace(PTRACE_SINGLESTEP, pid, ignored_addr, no_step_signal);
  if (result != 0) {
    perror("PTRACE_SINGLESTEP: ");
    abort();
  }
}
