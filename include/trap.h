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

/**
 * @brief Create and execute a new inferior processes.
 *
 * @param path The full path to the binary to execute.
 * @param argv A NULL terminated list of command line arguments to
 *   pass to the inferior process.
 * 
 * @return Returns a handle for the new inferior.
 *
 * The arguments to this function match those of execv(3)
 */
trap_inferior_t trap_inferior_exec(const char *path, char *const argv[]);

/**
 * @brief Set a breakpoint in `inferior` at `location`
 *
 * `trap_inferior_set_breakpoint` will set a breakpoint in the
 * inferior so that when the breakpoint is encountered the breakpoint
 * callback will be called.
 *
 * @param inferior The handle of the inferior process in which to set the
 *   breakpoint.
 * @param location The location at which to set the breakpoint.
 */
trap_breakpoint_t trap_inferior_set_breakpoint(trap_inferior_t inferior,
					       char *location);

/**
 * @brief Continue execution of `inferior`.
 *
 * Continue the execution of a stopped process.
 *
 * @param inferior The handle of a stopped inferior process which
 *   should continue execution.
 */
void trap_inferior_continue(trap_inferior_t inferior);

#endif /* !defined(_TRAP_H) */
