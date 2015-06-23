enable_testing()
set(LIBS trap)
include_directories(. ../include)

set(TESTS test_exec
          test_breakpoint
	        test_multi_breakpoint
          test_multiple_breakpoints)

foreach(TEST ${TESTS})
  add_executable(${TEST} ${TEST}.c)
  target_link_libraries(${TEST} ${LIBS})
  add_test(${TEST} ${CMAKE_CURRENT_BINARY_DIR}/${TEST})
endforeach(TEST)

add_subdirectory(inferiors)
