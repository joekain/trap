#include <assert.h>
#include <trap.h>
#include <unistd.h>

int g_breakpoint_count = 0;
trap_inferior_t g_inferior = 0;
trap_breakpoint_t g_bp = 0;

void breakpoint_callback(trap_inferior_t inferior, trap_breakpoint_t bp)
{
  assert(inferior == g_inferior);
  assert(bp == g_bp);

  g_breakpoint_count++;
}

int main()
{
  char *argv[1] = { 0 };

  trap_breakpoint_set_callback(breakpoint_callback);
  g_inferior = trap_inferior_exec("./inferiors/hello", argv);
  g_bp = trap_inferior_set_breakpoint(g_inferior, "main");
  trap_inferior_continue(g_inferior);

  assert(g_breakpoint_count == 1);

  return 0;
}
