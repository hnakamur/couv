SYS := $(shell gcc -dumpmachine)

HEADERS= \
  include/couv.h \
  include/couv-private/couv-private.h \
#  include/couv-private/couv-unix.h \
#  include/couv-private/couv-win.h \

OBJS= \
  src/auxlib.o \
  src/buf_alloc.o \
  src/buffer.o \
  src/fs.o \
  src/ipaddr.o \
  src/loop.o \
  src/tcp.o \
  src/udp.o \
  src/couv.o \

TARGET=couv_native.so
LIBUV_DIR=deps/uv
LIBUV_A=$(LIBUV_DIR)/libuv.a

CC=gcc

CFLAGS += -g
CFLAGS += --std=c89 -pedantic -Wall -Wextra -Wno-unused-parameter
CFLAGS += -Iinclude -Iinclude/couv-private -I$(LIBUV_DIR)/include

LDFLAGS += -L$(LIBUV_DIR) -luv -llua

LUA_E=

ifneq (, $(findstring linux, $(SYS)))
else ifneq (, $(findstring darwin, $(SYS)))
LUA_E=lua
LDFLAGS += -lpthread -bundle -undefined dynamic_lookup -framework CoreServices
else ifneq (, $(findstring mingw, $(SYS)))
endif

all: $(TARGET)

$(TARGET): $(LIBUV_A) $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(LIBUV_A):
	cd $(LIBUV_DIR) && make

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

src/auxlib.o: src/auxlib.c $(HEADERS)
src/buf_alloc.o: src/buf_alloc.c $(HEADERS)
src/buffer.o: src/buffer.c $(HEADERS)
src/couv.o: src/couv.c $(HEADERS)
src/fs.o: src/fs.c $(HEADERS)
src/ipaddr.o: src/ipaddr.c $(HEADERS)
src/loop.o: src/loop.c $(HEADERS)
src/tcp.o: src/tcp.c $(HEADERS)
src/udp.o: src/udp.c $(HEADERS)

.PHONY: test clean

test:
	$(LUA_E) tool/checkit test/test-buffer.lua
	$(LUA_E) tool/checkit test/test-*.lua

clean:
	-cd $(LIBUV_DIR) && make clean
	-rm -f $(TARGET) $(OBJS)
