#!/bin/bash
set -e

compilers=(
  g++-12
  g++-11
  g++-10
  g++-9

  clang++-14
  clang++-13
  clang++-12
  clang++-11

  icpx
)

cd ci

for compiler in "${compilers[@]}"; do
  build_dir="__build_${compiler}__"
  echo $build_dir
  mkdir $build_dir 2> /dev/null || true
  cd $build_dir && rm -rf *

  rm -rf CMakeCache.txt CMakeFiles

  cmake \
    -DCMAKE_CXX_COMPILER=$compiler \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake \
    -G Ninja \
    ..
  cmake --build .
  ctest . --output-on-failure -j22

  rm -rf CMakeCache.txt CMakeFiles

  cmake \
    -DCMAKE_CXX_COMPILER=$compiler \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake \
    -G Ninja \
    ..
  cmake --build .
  ctest . --output-on-failure -j22

  rm -rf CMakeCache.txt CMakeFiles

  # cmake \
  #   -DCMAKE_CXX_COMPILER=$compiler \
  #   -DCMAKE_BUILD_TYPE=Debug \
  #   -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake \
  #   -DCMAKE_CXX_STANDARD=20 \
  #   -G Ninja \
  #   ..
  # cmake --build .
  # ctest . --output-on-failure -j22

  # rm -rf CMakeCache.txt CMakeFiles

  # cmake \
  #   -DCMAKE_CXX_COMPILER=$compiler \
  #   -DCMAKE_BUILD_TYPE=Debug \
  #   -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake \
  #   -DCMAKE_CXX_STANDARD=20 \
  #   -G Ninja \
  #   ..
  # cmake --build .
  # ctest . --output-on-failure -j22

  cd ..
done

echo "Done with CI script!"

cd ..
