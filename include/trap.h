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

/**
 * @brief A handle to an inferior process under the control of the
 *   Trap debugger.
 *
 * A `trap_inferior_t` is used to refer to an inferior when making
 * Trap API calls.
 */
typedef int trap_inferior_t;

/**
 * @brief A handle to a breakpoint set in a specific inferior process
 *   under the control of the Trap debugger.
 *
 * A `trap_breakppint_t` handle is used to refer to a breakpoint in
 * Trap API calls.
 * 
 * A `trap_breakpoint_t` handle is only meaningful given a specific
 * inferior.  That is, the handle must always be used in conjunction
 * with a `trap_inferior_t`.
 */
typedef int trap_breakpoint_t;

/**
 * @brief A function pointer used as a callback.
 * 
 * @see trap_breakpoint_set_callback
 */
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
