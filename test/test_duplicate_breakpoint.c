#include <assert.h>
#include <trap.h>
#include <unistd.h>

trap_inferior_t g_inferior = 0;
trap_breakpoint_t g_bp_one = 0;
trap_breakpoint_t g_bp_two = 0;
int g_bp_one_count;
int g_bp_two_count;

void do_bp_one(trap_inferior_t inferior)
{
  g_bp_one_count++;
  if (g_bp_one_count == 1) {
    trap_inferior_remove_breakpoint(inferior, g_bp_one);
  }
}

void do_bp_two()
{
  g_bp_two_count++;
}

void breakpoint_callback(trap_inferior_t inferior, trap_breakpoint_t bp)
{
  if (bp == g_bp_one) {
    do_bp_one(inferior);
  } else if (bp == g_bp_two) {
    do_bp_two();
  }
}

int main()
{
  char *argv[1] = { 0 };

  trap_breakpoint_set_callback(breakpoint_callback);
  g_inferior = trap_inferior_exec("./inferiors/loop", argv);
  g_bp_one = trap_inferior_set_breakpoint(g_inferior, (char *)0x000000000040076d);
  g_bp_two = trap_inferior_set_breakpoint(g_inferior, (char *)0x000000000040076d);
  trap_inferior_continue(g_inferior);

  assert(g_bp_one_count == 1);
  assert(g_bp_two_count == 5);

  return 0;
}
