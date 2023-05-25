# Blaze

### Multi-threaded, CPU-based vector graphics rasterizer.

## Intro.
There is an article about this rasterizer at https://gasiulis.name/parallel-rasterization-on-cpu/.

## Supported systems.
Currently any system with POSIX threads. Which is all systems (including WebAssembly) except Windows. To add support for Windows, a new threading implementation and optimized bit manipulation functions (bit counting and trailing zero counting) that would compile to specific instructions are needed. Rasterizer depends on them being fast. I may add Windows support as soon as I get access to a Windows computer.

## Demo for WebAssembly.
There is a demo online https://gasiulis.name/parallel-rasterization-on-cpu/demo/.

Demo source is located in `Demo_wasm` directory. Run `build` script to compile it, then run `webserver` to start a local web server. It runs on port 80 so it may require a root password. Modify script to run server on any other port.

Building requires Emscripten and web server requires Python. Be aware that for threads to work on WebAssembly, `SharedArrayBuffer` needs to be available. It requires specific headers to be sent with responses from the web server.

## Demo for macOS.
A simple app demonstrating rasterizer is also included in the `Demo_macOS` directory. Double-clicking on the project file and pressing `⌘R` should do the trick.
