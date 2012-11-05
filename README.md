# couv - Coroutine based libuv wrapper library for lua

## Goals
* lua and native library (not a new language or a framework)
* use coroutine yield/resume for non-blocking I/O (to avoid callback hell).
  Thanks for this idea to https://github.com/grrrwaaa/luauv
* to be supported on Linux, OSX, Windows and iOS.
* works on the LuaJIT 2.0, Lua 5.2.1 (It does not work on Lua 5.1.5, but it
  should work on Lua 5.1.5 with Coco).
* embeddable in another event loop (not tested yet).

## Rules
* must be written in C89 (neither C++ nor C99) for MSVC compatibility.

## Tested environments
* luajit-2.0.0-rc1 on Ubuntu 12.04.1 x86_64
* homebrew lua 5.2.1 on Mac OS X Mountain Lion
* luajit-2.0.0-rc1 on Windows XP Pro SP3 + MinGW

## Build and test

```
make
make test
```

## Running tests with valgrind
```
valgrind --suppressions=/path/to/luajit-2.0/src/lj.supp luajit tool/checkit test/test-*.lua
```

## License
MIT License

## TODO
* Add benchmarks.
* fix errors reported by valgrind
* Implement more C/lua functions for functions in libuv.
