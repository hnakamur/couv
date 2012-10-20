# couv - Coroutine based libuv wrapper library for lua

## Goals
* lua and native library (not a new language or a framework)
* use coroutine yield/resume for non-blocking I/O (to avoid callback hell).
  Thanks for this idea to https://github.com/grrrwaaa/luauv
* to be supported on Linux, OSX, and Windows.
* works on the LuaJIT 2.0, Lua 5.2.1.
* embeddable in another event loop (not tested yet).

couv APIs are not object-oriented right now. Maybe I change to object-oriented APIs in the future if I find a simple way to implement inheritance of userdata with metatable and perfomance loss is negligible.

## Rules
* must be written in C89 (neither C++ nor C99) for MSVC compatibility.

## Tested environments
* luajit-2.0.0-beta11 on Ubuntu 12.04.1 x86_64
* homebrew lua 5.2.1 on Mac OS X Mountain Lion
* luajit-2.0.0-beta11 on Windows XP Pro SP3 + MinGW

## Build and test

```
make
make test
```

## License
MIT License

## TODO
* Add benchmarks.
* Implement more C/lua functions for functions in libuv.
* Decide whether or not we switch to object-oriented APIs in lua side.
