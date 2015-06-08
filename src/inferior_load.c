#include <debugger.h>
#include "inferior.h"
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


/* Used for ignored arguments */
static pid_t ignored_pid;
static void *ignored_ptr;

static void setup_inferior(const char *path, char *const argv[])
{
  ptrace(PTRACE_TRACEME, ignored_pid, ignored_ptr, ignored_ptr);
  execv(path, argv);
}

static void attach_to_inferior(pid_t pid)
{
  while(1) {

  }
}

void dbg_inferior_exec(const char *path, char *const argv[])
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
}
