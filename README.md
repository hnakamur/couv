# Yet Another Lua UV wrapper library

* lua native library (not a new language or a framework)
* written in C89 (neither C++ nor C99)
* to be supported on Linux, OSX, and Windows (tested on only Linux right now)
* works on the latest LuaJIT. I would like to support Lua 5.1 + Coco and Lua 5.2 too in the future.
* not object-oriented APIs. less is more. maybe change to object-oriented APIs in the futre if I find a simple way to implement inheritance of userdata with metatable.
* embeddable in another event loop
* use coroutine yield/resume for non-blocking I/O.
  Thanks for this idea to https://github.com/grrrwaaa/luauv
  (However this style may be changed in the future.)

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
* Rename this project. I think "couv" would be a good name.
  COroutine based libUV wrapper for lua.
* Add fixed size buffer memory allocator with a memory pool.
* Add benchmarks.
* Implement more C/lua functions for functions in libuv.
* Decide wether or not we switch to object-oriented APIs in lua side.
