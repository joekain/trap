#include <trap.h>
#include "inferior.h"
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


/* Used for ignored arguments */
static const pid_t ignored_pid;
static const void *ignored_ptr;
static const void *no_continue_signal = 0;

static void setup_inferior(const char *path, char *const argv[])
{
  ptrace(PTRACE_TRACEME, ignored_pid, ignored_ptr, ignored_ptr);
  execv(path, argv);
}

static void attach_to_inferior(pid_t pid)
{
  int status;
  waitpid(pid, &status, 0);

  if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
    return;
  } else {
    fprintf(stderr, "Unexpected status for inferior %d when attaching\n", pid);
    abort();
  }
}

trap_inferior_t trap_inferior_exec(const char *path, char *const argv[])
{
  pid_t result;

  do {
    result = fork();
    switch (result) {
    case 0:   // inferior
      setup_inferior(path, argv);
      break;
    case -1:  // error
      break;
    default:  // debugger
      attach_to_inferior(result);
      break;
    }
  } while (result == -1 && errno == EAGAIN);

  return result;
}

void trap_inferior_continue(trap_inferior_t inferior)
{
  pid_t pid = inferior;

  ptrace(PTRACE_CONT, pid, ignored_ptr, no_continue_signal);
  while(1) {
    int status;
    waitpid(pid, &status, 0);

    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
      struct user_regs_struct regs;

      trap_breakpoint_t bp = breakpoint_resolve(inferior);
      breakpoint_remove(inferior, bp);
      breakpoint_trigger_callback(inferior, bp);

      ptrace(PTRACE_GETREGS, pid, ignored_ptr, &regs);
      regs.rip -= 1;
      ptrace(PTRACE_SETREGS, pid, ignored_ptr, &regs);

      ptrace(PTRACE_CONT, pid, ignored_ptr, no_continue_signal);
    } else if (WIFEXITED(status)) {
      return;
    } else {
      fprintf(stderr, "Unexpected stop in trap_inferior_continue: 0x%x\n", status);
      abort();
    }
  }
}
