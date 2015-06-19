#include <trap.h>
#include "ptrace_util.h"
#include "breakpoint.h"
#include "inferior.h"
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static void setup_inferior(const char *path, char *const argv[])
{
  ptrace_util_traceme();
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

  ptrace_util_continue(pid);
  while(1) {
    int status;
    waitpid(pid, &status, 0);

    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
      breakpoint_handle(inferior);
    } else if (WIFEXITED(status)) {
      return;
    } else {
      fprintf(stderr, "Unexpected stop in trap_inferior_continue: 0x%x\n", status);
      abort();
    }
  }
}
