// Copyright © Advanced Micro Devices, Inc., or its affiliates.
//
// SPDX-License-Identifier:  MIT

#include "omp.h"

#include "common.h"

std::string bench_name = "omp_launch_latency";

bool run_bench() {
  {
    TimingCollector tc("empty kernel", nullptr, 0);

    while (!tc.done()) {
      Timing t(tc);
#pragma omp target teams ompx_bare num_teams(64) thread_limit(256)
      {
      }
    }
  }

  {
    TimingCollector tc("empty kernel with KLE", nullptr, 0);

    while (!tc.done()) {
      Timing t(tc);
#pragma omp target teams ompx_bare num_teams(64) thread_limit(256)             \
    ompx_dyn_cgroup_mem(1024)
      {
      }
    }
  }

  return true;
}
