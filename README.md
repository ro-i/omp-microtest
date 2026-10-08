Small tests and benchmarks for various parts of the LLVM OpenMP implementation.

Currently existing suites:
- `omp_shared_mem`: covering OpenMP allocations of shared (CPU/GPU) memory

How to build:
- set your C++ compiler in `CXX_BENCH` and your GPU arch in `OFFLOAD_ARCH`. You
  can do this via command line like `CXX_BENCH=my_clang++ make`, by editing
  `Makefile`, or by creating a `local.mk` file.
- run `make`/`make all` to build all existing suites or `make <suite>` to build
  a specific one.

The compiled suite binaries can be invoked with `-h` to show their specific
usage help.

Current test output format: running a suite prints one line for every test in
that suite. This line is structured as follows:
```
<suite name>|<test name>|n=...|i=...|d=... - min: ...; max: ...; avg: ...[ - OK|FAILED]
```
where
- `n`: the problem size in bytes used for this test. For example, the size of
  the allocations in `omp_shared_mem`. If a test has no notion of a problem
  size, it's 0.
- `i`: the number of iterations used for this test. This is either the
  configured number of benchmark iterations, or the number of iterations that
  resulted from running the test for a certain amount of time.
- `d`: the total amount of time used for this test
- `min`: the minimum time one iteration of this test took
- `max`: the maximum time one iteration of this test took
- `avg`: the average time one iteration of this test took (`avg` * `i` = `d`)
- (optional: an `OK` or `FAILED` indicator that shows whether the test
  succeeded or failed)
