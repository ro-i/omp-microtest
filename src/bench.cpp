// Copyright © Advanced Micro Devices, Inc., or its affiliates.
//
// SPDX-License-Identifier:  MIT

#include <unistd.h>

#include "common.h"

Config conf;

static void usage(std::string_view name) {
  std::cout << "usage: " << name << " [ARGS]\n"
            << "  -b N  benchmark iterations (default: auto-scaled such that "
               "the runtime per test is ~"
            << AUTO_SCALE_TIME << " second (min " << BENCH_MIN_ITERS
            << " iterations))\n"
            << "  -h    show this help\n"
            << "  -n    problem size (default: " << DEFAULT_PROBLEM_SIZE
            << ")\n"
            << "  -V    show compiler version used to compile this binary\n"
            << "  -w N  warmup iterations (default: 2)\n";
}

int main(int argc, char *const *argv) {
  int opt;

  while ((opt = getopt(argc, argv, "b:hn:Vw:")) != -1) {
    switch (opt) {
    case 'b':
      conf.bench_iters = std::stoi(optarg);
      conf.auto_scale = false;
      break;
    case 'h':
      usage(argv[0]);
      return EXIT_SUCCESS;
    case 'n':
      conf.problem_size = std::stoull(optarg);
      break;
    case 'V':
      std::cout << "clang " << __clang_version__ << "\n";
      return EXIT_SUCCESS;
    case 'w':
      conf.warmup_iters = std::stoi(optarg);
      break;
    default:
      usage(argv[0]);
      return EXIT_SUCCESS;
    }
  }

  return run_bench() ? EXIT_SUCCESS : EXIT_FAILURE;
}
