#include <assert.h>
#include <trap.h>
#include <unistd.h>

int g_main_count = 0;

trap_inferior_t g_inferior = 0;
trap_breakpoint_t g_bp_main = 0;

void breakpoint_callback(trap_inferior_t inferior, trap_breakpoint_t bp)
{
  assert(inferior == g_inferior);
  g_main_count++;
}

int main()
{
  char *argv[1] = { 0 };

  trap_breakpoint_set_callback(breakpoint_callback);
  g_inferior = trap_inferior_exec("./inferiors/loop", argv);
  g_bp_main = trap_inferior_set_breakpoint(g_inferior, (char *)0x0000000000001129);
  trap_inferior_remove_breakpoint(g_inferior, g_bp_main);
  trap_inferior_continue(g_inferior);

  assert(g_main_count == 0);

  return 0;
}
