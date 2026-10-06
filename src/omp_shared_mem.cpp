// Copyright © Advanced Micro Devices, Inc., or its affiliates.
//
// SPDX-License-Identifier:  MIT

#include "omp.h"

#include "common.h"

std::string bench_name = "omp_shared_mem";

extern "C" void *llvm_omp_target_alloc_shared(size_t, int);
extern "C" void llvm_omp_target_free_shared(void *, int);

bool run_bench() {
  const size_t n = conf.problem_size;
  const int device = omp_get_default_device();
  const int host = omp_get_initial_device();

  unsigned *buffer = static_cast<unsigned *>(malloc(n * sizeof(unsigned)));
  if (!buffer) {
    std::cerr << __FILE_NAME__ << ": malloc failed\n";
    return false;
  }
  for (size_t i = 0; i < n; ++i)
    buffer[i] = 2 * i;

  unsigned *shared = static_cast<unsigned *>(
      llvm_omp_target_alloc_shared(n * sizeof(unsigned), device));
  if (!shared) {
    std::cerr << __FILE_NAME__ << ": shared mem alloc failed\n";
    return false;
  }
  for (size_t i = 0; i < n; ++i)
    shared[i] = i;

  void *ret;
  size_t total_failures = 0;

  {
    size_t failures = 0;
    TimingCollector tc("target for loop", &failures);
    long iters = 0;

    while (!tc.done()) {
      {
        Timing t(tc);

#pragma omp target teams distribute parallel for device(device)                \
    is_device_ptr(shared)
        for (size_t j = 0; j < n; ++j)
          shared[j] += 1;
      }
      ++iters;
    }

    for (size_t i = 0; i < n; ++i)
      failures += (shared[i] != i + iters);
    total_failures += failures;
  }

  {
    size_t failures = 0;
    TimingCollector tc("target memset", &failures);

    // Filling the whole allocation.
    while (!tc.done()) {
      {
        Timing t(tc);
        ret = omp_target_memset(shared, 0, n * sizeof(int), device);
      }
      if (ret != shared)
        ++failures;
    }

    for (size_t i = 0; i < n; ++i)
      failures += (shared[i] != 0);
    total_failures += failures;
  }

  {
    size_t failures = 0;
    TimingCollector tc("copy host -> target shared", &failures);

    while (!tc.done()) {
      Timing t(tc);
      failures += (omp_target_memcpy(shared, buffer, n * sizeof(int), 0, 0,
                                     device, host) != 0);
    }

    total_failures += failures;
  }

  for (size_t i = 0; i < n; ++i)
    buffer[i] = 0;

  {
    size_t failures = 0;
    TimingCollector tc("copy target shared -> host", &failures);

    while (!tc.done()) {
      Timing t(tc);
      failures += (omp_target_memcpy(buffer, shared, n * sizeof(int), 0, 0,
                                     host, device) != 0);
    }

    for (size_t i = 0; i < n; ++i)
      failures += (buffer[i] != 2 * i);
    total_failures += failures;
  }

  free(buffer);
  llvm_omp_target_free_shared(shared, device);

  return !total_failures;
}
