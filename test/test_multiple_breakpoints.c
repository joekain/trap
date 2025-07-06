#include <assert.h>
#include <trap.h>
#include <unistd.h>

int g_foo_count = 0;
int g_main_count = 0;

trap_inferior_t g_inferior = 0;
trap_breakpoint_t g_bp_main = 0;
trap_breakpoint_t g_bp_foo  = 0;

void breakpoint_callback(trap_inferior_t inferior, trap_breakpoint_t bp)
{
  assert(inferior == g_inferior);

  if (bp == g_bp_main) {
    g_main_count++;
  } else if (bp == g_bp_foo) {
    g_foo_count++;
  }
}

int main()
{
  char *argv[1] = { 0 };

  trap_breakpoint_set_callback(breakpoint_callback);
  g_inferior = trap_inferior_exec("./inferiors/loop", argv);
  g_bp_main = trap_inferior_set_breakpoint(g_inferior, (char *)0x0000000000001138);
  g_bp_foo  = trap_inferior_set_breakpoint(g_inferior, (char *)0x0000000000001129);
  trap_inferior_continue(g_inferior);

  assert(g_main_count == 1);
  assert(g_foo_count == 5);

  return 0;
}
