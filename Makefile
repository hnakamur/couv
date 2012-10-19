SYS := $(shell gcc -dumpmachine)

HEADERS= \
  include/couv.h \
  include/couv-private/couv-private.h \
  include/couv-private/couv-unix.h \
  include/couv-private/couv-win.h \

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

TARGET_BASENAME=couv_native
LIBUV_DIR=deps/uv
LIBUV_A=$(LIBUV_DIR)/libuv.a
LIBUV_MAKE_OPT=

LUA_INC_DIR=/usr/local/include
LUA_LIB_DIR=/usr/local/lib

CC=gcc

CFLAGS += -g
CFLAGS += -Iinclude -Iinclude/couv-private \
	  -I$(LIBUV_DIR)/include -I$(LUA_INC_DIR)

LDFLAGS += -L$(LIBUV_DIR) -luv -L$(LUA_LIB_DIR) -llua

LUA_E=

ifneq (, $(findstring linux, $(SYS)))
TARGET=$(TARGET_BASENAME).so
LUA_E=luajit
LIBUV_MAKE_OPT += CFLAGS=-fPIC
CFLAGS += -fPIC -D_XOPEN_SOURCE=500 -D_GNU_SOURCE
CFLAGS += --std=c89 -pedantic -Wall -Wextra -Wno-unused-parameter
LDFLAGS += -shared -Wl,-soname,$(TARGET) -lpthread -lrt -lm
else ifneq (, $(findstring darwin, $(SYS)))
TARGET=$(TARGET_BASENAME).so
LUA_E=lua
CFLAGS += --std=c89 -pedantic -Wall -Wextra -Wno-unused-parameter
LDFLAGS += -lpthread -bundle -undefined dynamic_lookup -framework CoreServices
else ifneq (, $(findstring mingw, $(SYS)))
TARGET=$(TARGET_BASENAME).dll
LUA_E=lua
CFLAGS += --std=gnu89 -D_WIN32_WINNT=0x0600
LDFLAGS += -shared -Wl,--export-all-symbols -lws2_32 -lpsapi -liphlpapi
endif

all: $(TARGET)

$(TARGET): $(LIBUV_A) $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(LIBUV_A):
	cd $(LIBUV_DIR) && make $(LIBUV_MAKE_OPT)

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
