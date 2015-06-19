include_directories(. ../include)
add_library(trap inferior_load.c breakpoint.c ptrace_util.c)
