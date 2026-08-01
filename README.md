Concurrent Memory Pool Allocator

Simple student project implementing a fixed-size, lock-free memory pool in C++20.

Files:
- main.cpp — allocator and benchmark
- report.txt — short project analysis (architectural analysis, concurrency safety, performance trade offs, memory safety audit)

Build and run (MSVC Developer Command Prompt for x64):

1. Open "Developer Command Prompt for VS 2022" (x64)
2. Build:

```bat
cl /nologo /std:c++20 /W4 /EHsc main.cpp /Fe:pool.exe
```

3. Run:

```bat
pool.exe
```

Optional: build with AddressSanitizer (if supported by your MSVC):

```bat
cl /nologo /std:c++20 /W4 /EHsc /fsanitize=address /Zi main.cpp /Fe:pool_asan.exe
pool_asan.exe
```

Notes:
- This project is a small assignment submission. Do not include build artifacts when pushing to Git.
- The allocator is designed for fixed-size allocations and does not support dynamic growth or variable-sized blocks.