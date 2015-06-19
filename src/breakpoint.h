#if !defined(_TRAP_BREAKPOINT_H)
#define _TRAP_BREAKPOINT_H
#include "inferior.h"

enum inferior_state_t breakpoint_handle(trap_inferior_t inferior, enum inferior_state_t state);

#endif /* _TRAP_BREAKPOINT_H */
