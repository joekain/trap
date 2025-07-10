#include <trap.h>
#include "ptrace_util.h"
#include "breakpoint.h"
#include "inferior.h"
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <sys/personality.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static inferior_t g_inferior;

static void setup_inferior(const char *path, char *const argv[])
{
  unsigned long old = personality(0xFFFFFFFF);
  if (personality(old | ADDR_NO_RANDOMIZE) < 0) {
    perror("Failed to set personality:");
  }
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

  g_inferior.pid = result;
  g_inferior.state = INFERIOR_STOPPED;
  return result;
}

static inferior_t *inferior_resolve(trap_inferior_t inferior)
{
  assert(inferior == g_inferior.pid);
  return &g_inferior;
}

void trap_inferior_continue(trap_inferior_t inferior)
{
  inferior_t *inf = inferior_resolve(inferior);

  inf->state = INFERIOR_RUNNING;
  ptrace_util_continue(inf->pid);
  while(1) {
    int status;
    waitpid(inf->pid, &status, 0);

    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
      inf->state = breakpoint_handle(inferior, inf->state);
    } else if (WIFEXITED(status)) {
      return;
    } else {
      fprintf(stderr, "Unexpected stop in trap_inferior_continue: 0x%x\n", status);
      abort();
    }
  }
}
