#include <trap.h>
#include "ptrace_util.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <breakpoint.h>

struct breakpoint_t {
  uintptr_t target_address;
  uintptr_t aligned_address;
  uintptr_t original_breakpoint_word;
};
typedef struct breakpoint_t breakpoint_t;

#define MAX_BREAKPOINTS 100
static int g_num_breakpoints;
static breakpoint_t g_breakpoints[MAX_BREAKPOINTS];

static trap_breakpoint_callback_t g_callback;

void trap_breakpoint_set_callback(trap_breakpoint_callback_t callback)
{
  g_callback = callback;
}

static void breakpoint_set(trap_inferior_t inferior, breakpoint_t *bp)
{
  const uintptr_t int3_opcode = 0xCC;
  pid_t pid = inferior;
  uintptr_t target_offset = bp->target_address - bp->aligned_address;

  uintptr_t modified_word;
  modified_word = bp->original_breakpoint_word;
  modified_word &= ~(0xFFUL << (target_offset * 8));
  modified_word |= int3_opcode << (target_offset * 8);
  ptrace_util_poke_text(pid, bp->aligned_address, modified_word);
}

static breakpoint_t *breakpoint_allocate()
{
  int index = g_num_breakpoints;
  g_num_breakpoints++;

  assert(index < MAX_BREAKPOINTS);
  return &g_breakpoints[index];
}

trap_breakpoint_t trap_inferior_set_breakpoint(trap_inferior_t inferior,
                                               char *location)
{
  const uintptr_t target_address = (uintptr_t)location;
  uintptr_t aligned_address = target_address & ~(0x7UL);
  breakpoint_t *bp = breakpoint_allocate();

  bp->original_breakpoint_word = ptrace_util_peek_text(inferior,
						       aligned_address);
  bp->target_address = target_address;
  bp->aligned_address = aligned_address;
  
  breakpoint_set(inferior, bp);

  return bp;
}

static void breakpoint_trigger_callback(trap_inferior_t inferior,
					trap_breakpoint_t handle)
{
  (*g_callback)(inferior, handle);
}

static trap_breakpoint_t breakpoint_resolve(trap_inferior_t inferior)
{
  return &g_breakpoint;
}

static void breakpoint_remove(trap_inferior_t inferior,
			      trap_breakpoint_t handle)
{
  breakpoint_t *bp = (breakpoint_t *)handle;
  pid_t inferior_pid = inferior;

  ptrace_util_poke_text(inferior_pid, bp->aligned_address,
			bp->original_breakpoint_word);
}

static void step_over_breakpoint(trap_inferior_t inferior)
{
  struct user_regs_struct regs;
  pid_t pid = inferior;

  trap_breakpoint_t bp = breakpoint_resolve(inferior);
  breakpoint_remove(inferior, bp);
  breakpoint_trigger_callback(inferior, bp);

  ptrace_util_get_regs(pid, &regs);
  ptrace_util_set_instruction_pointer(pid, regs.rip - 1);

  ptrace_util_single_step(pid);
}

static void finish_breakpoint(trap_inferior_t inferior)
{
  breakpoint_t *bp = breakpoint_resolve(inferior);
  breakpoint_set(inferior, bp);
  ptrace_util_continue(inferior);
}

enum inferior_state_t breakpoint_handle(trap_inferior_t inferior, enum inferior_state_t state)
{
  switch(state) {
    case INFERIOR_RUNNING:
      step_over_breakpoint(inferior);
      return INFERIOR_SINGLE_STEPPING;

    case INFERIOR_SINGLE_STEPPING:
      finish_breakpoint(inferior);
      return INFERIOR_RUNNING;

    default:
      abort();
  }
}
