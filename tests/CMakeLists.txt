# Copyright (c) 2022 Christian Mazakas
# 
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

function(stl2_add_test test_name)
  add_executable(${test_name} "${test_name}.cpp")
  target_include_directories(${test_name} PRIVATE vendor)

  target_link_libraries(${test_name} PRIVATE libless)
  set_target_properties(${test_name} PROPERTIES FOLDER "Test")
  add_test(${test_name} ${test_name})
endfunction()

function(stl2_add_compile_fail_test test_name)
  add_executable(${test_name} EXCLUDE_FROM_ALL  "${test_name}.cpp")
  target_link_libraries(${test_name} PRIVATE libless)
  add_test(NAME ${test_name} COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target ${test_name} --config $<CONFIG>)
  set_tests_properties(${test_name} PROPERTIES WILL_FAIL TRUE RUN_SERIAL TRUE)
endfunction()


stl2_add_test(compile)
stl2_add_test(construct)
stl2_add_test(construct_exception)
stl2_add_test(iterator_constructor)

stl2_add_compile_fail_test(initializer_list_constructor_fail)
