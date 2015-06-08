#include <debugger.h>

int main()
{
  char *argv[1] = { 0 };
  dbg_inferior_exec("./hello", argv);

  return 0;
}
