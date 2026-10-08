Small tests and benchmarks for various parts of the LLVM OpenMP implementation.

Currently existing suites:
- `omp_shared_mem`: covering OpenMP allocations of shared (CPU/GPU) memory

How to build:
Set your C++ compiler in `CXX_BENCH` and your GPU arch in `OFFLOAD_ARCH`. You
can do this via command line like `CXX_BENCH=my_clang++ make`, by editing
`Makefile`, or by creating a `local.mk` file.
Run `make`/`make all` to build all existing suites or `make <suite>` to build a
specific one. The compiled suite binaries can be invoked with `-h` to show
their specific usage help.
