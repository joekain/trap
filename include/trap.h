#if !defined(_TRAP_H)
#define _TRAP_H

/**
 * @file trap.h
 * @brief Trap main header file.
 * @author Joseph Kain
 * @date June 18, 2015
 *
 * Trap is a lightweight debugger controlled by the API described in
 * this file.
 * @see http://system.joekain.com/debugger/
 */

typedef int trap_inferior_t;
typedef int trap_breakpoint_t;
typedef void (*trap_breakpoint_callback_t)(trap_inferior_t, trap_breakpoint_t);

/**
 * @brief Set a function as the global breakpoint.
 *
 * @param callback A function that will be called whenever a
 *    breakpoint is encountered.
 */
void trap_breakpoint_set_callback(trap_breakpoint_callback_t callback);

void dbg_inferior_exec(const char *path, char *const argv[]);

#endif /* !defined(_TRAP_H) */
