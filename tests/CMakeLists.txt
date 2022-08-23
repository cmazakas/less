# Copyright (c) 2022 Christian Mazakas
#
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

function(libless_add_test test_name)
  if (UNIX)
    set(LIBLESS_MEMCHECK_COMMAND "valgrind")
    set(LIBLESS_MEMCHECK_ARGS "--leak-check=full --error-exitcode=1")
  elseif(WIN32)
    set(LIBLESS_MEMCHECK_COMMAND "drmemory")
    set(LIBLESS_MEMCHECK_ARGS "")
  else()
    set(LIBLESS_MEMCHECK_COMMAND "")
    set(LIBLESS_MEMCHECK_ARGS "")
  endif()

  set(memcheck_command "${LIBLESS_MEMCHECK_COMMAND} ${LIBLESS_MEMCHECK_ARGS}")
  separate_arguments(memcheck_command)

  add_executable(${test_name} "${test_name}.cpp")
  target_include_directories(${test_name} PRIVATE vendor)

  target_link_libraries(${test_name} PRIVATE libless)
  set_target_properties(${test_name} PROPERTIES FOLDER "Test")
  add_test(NAME ${test_name} COMMAND ${memcheck_command} ./${test_name})
endfunction()

function(stl2_add_compile_fail_test test_name)
  add_executable(${test_name} EXCLUDE_FROM_ALL "${test_name}.cpp")
  target_link_libraries(${test_name} PRIVATE libless)
  add_test(NAME ${test_name} COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${test_name} --config $<CONFIG>)
  set_tests_properties(${test_name} PROPERTIES WILL_FAIL TRUE RUN_SERIAL TRUE)
endfunction()


libless_add_test(type_traits)
libless_add_test(compile)
libless_add_test(construct)
libless_add_test(construct_exception)
libless_add_test(iterator_constructor)
libless_add_test(at)
libless_add_test(front)
libless_add_test(back)
libless_add_test(iterators)
libless_add_test(assign)
libless_add_test(resize_and_overwrite)
libless_add_test(shrink_to_fit)
libless_add_test(max_size)
libless_add_test(insert)
libless_add_test(emplace)
libless_add_test(erase)
libless_add_test(push_back)
libless_add_test(emplace_back)

stl2_add_compile_fail_test(initializer_list_constructor_fail)
