project(couv C)
cmake_minimum_required(VERSION 2.6)
include(dist.cmake)
include(checkit.cmake)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_BUILD_TYPE Debug)

set(LIBS
  lib/couv.lua
)
install_lua_module(couv ${LIBS})
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_SOURCE_DIR}/lib/couv.lua ${CMAKE_BINARY_DIR}/couv.lua)

add_subdirectory(deps/uv)
include_directories(include include/couv-private deps/uv/include)
set(SRCS
  src/auxlib.c
  src/buf_alloc.c
  src/buffer.c
  src/fs.c
  src/ipaddr.c
  src/loop.c
  src/tcp.c
  src/udp.c
  src/couv.c
)
install_lua_module(couv_native ${SRCS})
set_property(SOURCE ${SRCS} PROPERTY
  COMPILE_DEFINITIONS _XOPEN_SOURCE=500 _GNU_SOURCE)
set_property(SOURCE ${SRCS} PROPERTY
  COMPILE_FLAGS "--std=c89 -pedantic -Wall -Wextra -Wno-unused-parameter")
add_checkit_test(test/*.lua)
target_link_libraries(couv_native uv pthread rt)
