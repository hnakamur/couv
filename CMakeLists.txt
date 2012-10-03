project(yaluv C)
cmake_minimum_required(VERSION 2.6)
include(dist.cmake)
include(checkit.cmake)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(deps/uv)
include_directories(deps/uv/include)
set(SRCS
  src/yaluv.c
)
install_lua_module(yaluv ${SRCS})
set_property(SOURCE ${SRCS} PROPERTY
  COMPILE_DEFINITIONS _XOPEN_SOURCE=500)
set_property(SOURCE ${SRCS} PROPERTY
  COMPILE_FLAGS "--std=c89 -pedantic -Wall -Wextra -Wno-unused-parameter")
add_checkit_test(test/*.lua)
target_link_libraries(yaluv uv rt)
