

# General
cmake_minimum_required(VERSION 3.18)
set(CMAKE_CXX_STANDARD 23)
project(AvlTree)
include(ExternalProject)
set(CMAKE_BUILD_PARALLEL_LEVEL 8)

# Main executable
add_executable(main
src/main.cpp
src/main_tests.cpp
)

# catch2
set(CATCH2_DIR ${CMAKE_BINARY_DIR}/catch2)

ExternalProject_add(catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2
  GIT_TAG 0321d2f
  PREFIX ${CMAKE_BINARY_DIR}/catch2_git
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CATCH2_DIR}
)

include_directories(${CATCH2_DIR}/include)
target_link_libraries(main ${CATCH2_DIR}/lib64/libCatch2Main.a)
target_link_libraries(main ${CATCH2_DIR}/lib64/libCatch2.a)

add_dependencies(main catch2)
