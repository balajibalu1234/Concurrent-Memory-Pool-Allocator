Concurrent Memory Pool Allocator

I built this as a small C++20 project to explore how a fixed-size, lock-free allocator can reuse memory without relying on mutexes. The main goal was to keep the design simple while still showing the core ideas behind lock-free allocation, thread contention, and benchmark comparison.

What is in this repository
- main.cpp — allocator implementation and a simple benchmark
- report.txt — project notes and design observations

How it works
- The pool keeps one large buffer in memory and splits it into fixed-size chunks.
- Free chunks are stored in a lock-free linked list.
- Allocation removes the head of the list with compare-and-swap.
- Deallocation pushes a chunk back onto the head.
- I also added a tiny thread-local cache so each thread can reuse recently freed blocks before touching the shared list again.

Build and run on Windows

1. Open the MSVC Developer Command Prompt for x64.
2. Build the project:

```bat
cl /nologo /std:c++20 /W4 /EHsc main.cpp /Fe:pool.exe
```

3. Run it:

```bat
pool.exe
```

Optional ASan build

```bat
cl /nologo /std:c++20 /W4 /EHsc /fsanitize=address /Zi main.cpp /Fe:pool_asan.exe
pool_asan.exe
```

Notes
- This project is meant for learning and assignment-style work.
- The allocator is optimized for fixed-size allocations only.
- It does not support dynamic resizing or variable-size blocks.