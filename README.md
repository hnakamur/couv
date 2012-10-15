# couv - Coroutine based libuv wrapper library for lua

* lua and native library (not a new language or a framework)
* written in C89 (neither C++ nor C99)
* to be supported on Linux, OSX, and Windows (tested on only Linux right now)
* works on the latest LuaJIT. I would like to support Lua 5.1 + Coco and Lua 5.2 too in the future.
* not object-oriented APIs right now. maybe change to object-oriented APIs in the futre if I find a simple way to implement inheritance of userdata with metatable.
* embeddable in another event loop (not tested yet).
* use coroutine yield/resume for non-blocking I/O.
  Thanks for this idea to https://github.com/grrrwaaa/luauv

## Build and test

```
mkdir out
cd out
cmake ..
make
make test
```

## License
MIT License

## TODO
* Add fixed size buffer memory allocator with a memory pool.
* Add benchmarks.
* Implement more C/lua functions for functions in libuv.
* Decide wether or not we switch to object-oriented APIs in lua side.
