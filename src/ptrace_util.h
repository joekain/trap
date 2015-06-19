#if !defined(_TRAP_PTRACE_UTIL_H)
#define _TRAP_PTRACE_UTILT_H

#include <sys/types.h>
#include <sys/user.h>
#include <stdint.h>

void ptrace_util_traceme();
void ptrace_util_continue(pid_t pid);
long ptrace_util_peek_text(pid_t pid, unsigned long target_address);
void ptrace_util_poke_text(pid_t pid, unsigned long target_address,
			   unsigned long data);
void ptrace_util_get_regs(pid_t pid, struct user_regs_struct *regs);
void ptrace_util_set_regs(pid_t pid, struct user_regs_struct *regs);
void ptrace_util_set_instruction_pointer(pid_t pid, uintptr_t ip);

#endif /* _TRAP_PTRACE_UTIL_H */
