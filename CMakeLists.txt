# Copyright (c) 2022 Christian Mazakas
# 
# Distributed under the Boost Software License, Version 1.0. (See
# accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.22)
project(libless VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "Minimum required C++ standard")
if (CMAKE_CXX_STANDARD LESS 17)
  message(FATAL_ERROR "Minimum required C++ standard is currently c++17")
endif()

add_library(libless INTERFACE)
target_include_directories(libless INTERFACE include)

include(CTest)
add_subdirectory(tests)
