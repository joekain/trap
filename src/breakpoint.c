#include <trap.h>
#include "ptrace_util.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <breakpoint.h>

enum breakpoint_state_t {
  BREAKPOINT_UNALLOCATED,
  BREAKPOINT_ACTIVE,
  BREAKPOINT_DEFERRED_REMOVE
};

struct breakpoint_t {
  uintptr_t target_address;
  uintptr_t aligned_address;
  uintptr_t original_breakpoint_word;
  enum breakpoint_state_t state;
};

typedef struct breakpoint_t breakpoint_t;

#define MAX_BREAKPOINTS 100
static int g_num_breakpoints;
static breakpoint_t g_breakpoints[MAX_BREAKPOINTS];

static trap_breakpoint_callback_t g_callback;

static breakpoint_t *find_breakpoint_with_target_address(uintptr_t address)
{
  for (int i = 0; i < g_num_breakpoints; i++) {
    if (g_breakpoints[i].target_address == address) {
      return &g_breakpoints[i];
    }
  }

  return NULL;
}

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
  g_breakpoints[index].state = BREAKPOINT_ACTIVE;
  return &g_breakpoints[index];
}

trap_breakpoint_t trap_inferior_set_breakpoint(trap_inferior_t inferior,
                                               char *location)
{
  const uintptr_t target_address = (uintptr_t)location;
  uintptr_t aligned_address = target_address & ~(0x7UL);
  breakpoint_t *bp = breakpoint_allocate();

  breakpoint_t *existing = find_breakpoint_with_target_address(target_address);
  if (existing) {
    bp->original_breakpoint_word = existing->original_breakpoint_word;
  } else {
    bp->original_breakpoint_word = ptrace_util_peek_text(inferior,
						                                             aligned_address);
  }
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
  uintptr_t ip = ptrace_util_get_instruction_pointer(inferior) - 1;
  trap_breakpoint_t result = find_breakpoint_with_target_address(ip);
  assert(result);
  return result;
}

static void breakpoint_remove(trap_inferior_t inferior,
			      trap_breakpoint_t handle)
{
  breakpoint_t *bp = (breakpoint_t *)handle;
  pid_t inferior_pid = inferior;

  ptrace_util_poke_text(inferior_pid, bp->aligned_address,
			bp->original_breakpoint_word);
}

static void do_callbacks(trap_inferior_t inferior, breakpoint_t *bp)
{
  for (int i = 0; i < g_num_breakpoints; i++) {
    if (g_breakpoints[i].target_address == bp->target_address) {
      breakpoint_trigger_callback(inferior, &g_breakpoints[i]);
    }
  }
}

static void step_over_breakpoint(trap_inferior_t inferior, breakpoint_t *bp)
{
  uintptr_t ip;
  pid_t pid = inferior;

  breakpoint_remove(inferior, bp);

  ip = ptrace_util_get_instruction_pointer(pid);
  ptrace_util_set_instruction_pointer(pid, ip - 1);

  ptrace_util_single_step(pid);
}

static void do_deferred_removals(trap_inferior_t inferior)
{
  for (int i = 0; i < g_num_breakpoints; i++) {
    if (g_breakpoints[i].state == BREAKPOINT_DEFERRED_REMOVE) {
      g_breakpoints[i].target_address = 0;
      g_breakpoints[i].aligned_address = 0;
      g_breakpoints[i].original_breakpoint_word = 0;
      g_breakpoints[i].state = BREAKPOINT_UNALLOCATED;
    }
  }
}

static void finish_breakpoint(trap_inferior_t inferior, breakpoint_t *bp)
{
  uintptr_t target_address = bp->target_address;

  do_deferred_removals(inferior);
  breakpoint_t *new_bp = find_breakpoint_with_target_address(target_address);
  if (new_bp) {
    breakpoint_set(inferior, new_bp);
  }
  ptrace_util_continue(inferior);
}

static void start_breakpoint(trap_inferior_t inferior,
                             breakpoint_t *bp)
{
  do_callbacks(inferior, bp);
  step_over_breakpoint(inferior, bp);
}

enum inferior_state_t breakpoint_handle(trap_inferior_t inferior, enum inferior_state_t state)
{
  trap_breakpoint_t bp = breakpoint_resolve(inferior);

  switch(state) {
    case INFERIOR_RUNNING:
      start_breakpoint(inferior, bp);
      return INFERIOR_SINGLE_STEPPING;

    case INFERIOR_SINGLE_STEPPING:
      finish_breakpoint(inferior, bp);
      return INFERIOR_RUNNING;

    default:
      abort();
  }
}

void trap_inferior_remove_breakpoint(trap_inferior_t inferior,
		                                 trap_breakpoint_t handle)
{
  breakpoint_t *bp = (breakpoint_t *)handle;
  enum inferior_state_t state = inferior_get_state(inferior);

  assert(bp->state == BREAKPOINT_ACTIVE);

  switch(state) {
    case INFERIOR_STOPPED:
      breakpoint_remove(inferior, bp);
      bp->state = BREAKPOINT_UNALLOCATED;
      break;

    case INFERIOR_RUNNING:
      bp->state = BREAKPOINT_DEFERRED_REMOVE;
      break;

    default:
      abort();
  }
}
