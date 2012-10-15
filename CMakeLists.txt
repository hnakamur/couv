project(yaluv C)
cmake_minimum_required(VERSION 2.6)
include(dist.cmake)
include(checkit.cmake)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_BUILD_TYPE Debug)

set(LIBS
  lib/yaluv.lua
)
install_lua_module(yaluv ${LIBS})
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_SOURCE_DIR}/lib/yaluv.lua ${CMAKE_BINARY_DIR}/yaluv.lua)

add_subdirectory(deps/uv)
include_directories(include include/yaluv-private deps/uv/include)
set(SRCS
  src/auxlib.c
  src/buffer.c
  src/fs.c
  src/ipaddr.c
  src/loop.c
  src/tcp.c
  src/udp.c
  src/yaluv.c
)
install_lua_module(yaluv_native ${SRCS})
set_property(SOURCE ${SRCS} PROPERTY
  COMPILE_DEFINITIONS _XOPEN_SOURCE=500 _GNU_SOURCE)
set_property(SOURCE ${SRCS} PROPERTY
  COMPILE_FLAGS "--std=c89 -pedantic -Wall -Wextra -Wno-unused-parameter")
add_checkit_test(test/*.lua)
target_link_libraries(yaluv_native uv pthread rt)
