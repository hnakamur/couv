# Yet Another Lua UV wrapper library

* lua native library (not a new language or a framework)
* written in C89 (neither C++ nor C99)
* tested on Linux, OSX, and Windows
* embeddable in another event loop
* use coroutine yield/resume for non-blocking I/O
  Thanks for this idea to https://github.com/grrrwaaa/luauv
  However this style may be changed in the future.

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
