# ShadowHTTP (In Development)

A high-performance, bare-metal HTTP web server written from scratch in C++. 

This project is an exploration of low-level systems programming, Linux TCP/IP networking, and memory-safe architecture without relying on high-level frameworks or bloated abstractions. It is built directly on top of POSIX sockets and raw system calls.

The core focus of this server is performance and low memory foortprint achieved by avoiding the use of dynamic memory allocation and in the future will add multithreading and use an epoll based event loop.

## Current Features
- TCP Socket wrapper (IPv4/IPv6 resolution).
- Zero-Copy HTTP Parser utilizing a 16KB flat buffer to process network streams and detect `\r\n\r\n` boundaries without allocating heap memory.
- Zero-dependency (C++ Standard Library & POSIX APIs only).

## Roadmap / Known Issues
- Implement HTTP Request Line tokenizer (Method, URI, HTTP Version).
- Migrate from blocking `accept()` and `recv()` to an `epoll` based event loop for concurrent connection handling.
- Add a multithreaded worker pool and task queue to decouple connection handling from request processing.

## Building and Running
Compiled with strict warnings and optimizations (`-O3`, `-flto`, `-Werror`).

```bash
# Clone the repository
git clone https://github.com/ISMAILSHADOW/ShadowHTTP.git
cd ShadowHTTP

# Build the server
./build.sh release

# Run the server
./build/bin/server