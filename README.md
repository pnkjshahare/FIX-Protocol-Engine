# High-Performance FIX Protocol Engine (C++20)

> A low-latency FIX 4.4 parser and encoder built from scratch in C++20 with a focus on performance, memory efficiency, and systems programming concepts used in High-Frequency Trading (HFT).

---

## 🎯 Project Goal

The goal of this project is to build a production-inspired FIX engine capable of parsing, validating, encoding, and transmitting FIX messages with minimal latency and memory allocations.

This project is designed as a learning exercise to understand how trading applications communicate with exchanges and to develop the low-level systems programming skills expected in HFT environments.

---

# Learning Objectives

By completing this project, I aim to understand:

* FIX Protocol fundamentals
* Low-latency message parsing
* Efficient message serialization
* Memory management
* Zero-copy programming
* TCP socket programming
* Performance optimization
* Benchmarking and profiling
* Session management
* Cache-friendly programming

---

# Project Roadmap

## Phase 1 — Learn FIX Protocol

### Functionality

* Study the FIX 4.4 specification
* Understand Tag=Value format
* Understand SOH (`\x01`) delimiter
* Learn Session Layer vs Application Layer
* Learn common FIX messages

### Messages to Support

* Logon
* Heartbeat
* Test Request
* Logout
* New Order Single
* Order Cancel Request
* Execution Report
* Order Cancel Reject
* Reject

### Concepts Learned

* Electronic trading workflow
* Exchange communication
* Financial messaging standards

---

## Phase 2 — FIX Parser

### Functionality

* Parse raw FIX messages
* Extract tags and values
* Parse directly from input buffer
* Handle malformed messages
* Store parsed fields efficiently

### Concepts Learned

* Pointer arithmetic
* Character buffer processing
* String parsing
* Buffer management
* Zero-copy parsing
* Branch prediction

---

## Phase 3 — FIX Encoder

### Functionality

* Generate FIX messages
* Serialize C++ objects
* Calculate BodyLength (Tag 9)
* Calculate Checksum (Tag 10)

### Concepts Learned

* Serialization
* Buffer writing
* Integer-to-string conversion
* Protocol encoding

---

## Phase 4 — Message Validation

### Functionality

* Validate required tags
* Validate message format
* Reject invalid messages

### Concepts Learned

* Protocol validation
* Error handling
* Defensive programming

---

## Phase 5 — Fast Field Lookup

### Functionality

* Efficient tag lookup
* Compare different lookup strategies

### Possible Implementations

* `switch`
* Lookup table
* `unordered_map`
* Perfect hashing (optional)

### Concepts Learned

* Hash tables
* Cache-friendly data structures
* Lookup optimization

---

## Phase 6 — Zero-Copy Parsing

### Functionality

* Eliminate unnecessary string copies
* Parse directly from receive buffer

### Techniques

* `std::string_view`
* Raw pointers

### Concepts Learned

* Memory ownership
* Object lifetime
* Allocation-free programming

---

## Phase 7 — Memory Pool

### Functionality

* Reuse message objects
* Reduce heap allocations

### Concepts Learned

* Memory pool
* Object pool
* Cache locality
* Custom allocators

---

## Phase 8 — Benchmarking

### Functionality

Measure

* Throughput
* Latency
* CPU cycles
* Memory allocations

### Tools

* Google Benchmark
* Linux `perf`
* Valgrind

### Concepts Learned

* Performance profiling
* Bottleneck analysis

---

## Phase 9 — TCP Networking

### Functionality

* Send FIX messages
* Receive FIX messages
* Non-blocking communication

### Concepts Learned

* BSD sockets
* TCP/IP
* `send()`
* `recv()`
* Non-blocking I/O

---

## Phase 10 — Session Layer

### Functionality

Implement

* Logon
* Heartbeat
* Logout
* Sequence Numbers
* Resend Request

### Concepts Learned

* State machines
* Session management
* Connection lifecycle
* Recovery mechanisms

---

# Performance Goals

* Zero-copy parsing
* Minimal heap allocations
* Cache-friendly data structures
* Low-latency encoding
* Efficient field lookup
* High message throughput

---

# Tech Stack

* C++20
* CMake
* STL
* Google Benchmark
* Google Test (or Catch2)
* Linux
* Git
* perf
* Valgrind

---

# Skills Gained

* Modern C++
* Systems Programming
* Memory Management
* Low-Latency Programming
* FIX Protocol
* Network Programming
* Performance Optimization
* Benchmarking
* Cache Optimization
* Profiling

---

# Future Improvements

* FIX Dictionary
* Multiple FIX Versions
* Asynchronous Networking
* Multi-threaded Processing
* Custom Memory Allocator
* Binary Protocol Support
* Market Data Messages
* TLS Support
* Replay & Recovery
* Order Routing

---

# Related Projects

After completing this project, the next components of my HFT learning roadmap are:

* High-Performance Order Book
* Matching Engine
* Lock-Free Ring Buffer
* Market Data Feed Handler
* Event Scheduler
* Custom Memory Allocator
* Backtesting Engine

Together, these projects will form a complete low-latency trading system built entirely from scratch.

---

# References

* FIX Protocol 4.4 Specification
* Modern C++ (C++20)
* Linux Performance Tools (`perf`)
* Google Benchmark
* High-Performance Systems Programming Resources
