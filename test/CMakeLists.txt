enable_testing()
set(LIBS trap)
include_directories(. ../include)
add_executable(test_exec test_exec.c)
target_link_libraries(test_exec ${LIBS})
add_test(test_exec ${CMAKE_CURRENT_BINARY_DIR}/test_exec)

add_subdirectory(inferiors)
