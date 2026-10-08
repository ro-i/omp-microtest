// Copyright © Advanced Micro Devices, Inc., or its affiliates.
//
// SPDX-License-Identifier:  MIT

#pragma once

#include <algorithm>
#include <chrono>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <string_view>
#include <tuple>
#include <vector>

// Benchmark minimum number of measured iterations (as a lower bound in case the
// test is so slow that it wouldn't get enough iterations in the auto-scale
// timeframe)
#define BENCH_MIN_ITERS 10
// Auto-scale timeframe in seconds. Benchmarks will be repeated until they reach
// at least this amount of seconds.
#define AUTO_SCALE_TIME 1.0

#define duration_cast(x)                                                       \
  std::chrono::duration_cast<std::chrono::duration<double>>(x)
using Clock = std::chrono::steady_clock;

static constexpr size_t DEFAULT_PROBLEM_SIZE = 1 << 28; // 268,435,456

// Benchmark declarations.
// These are expected to be defined in the source file for that benchmark.
extern std::string bench_name;
extern bool run_bench();

template <typename T> inline T saturating_add(T a, T b) {
  T max = std::numeric_limits<T>::max();
  return max - a < b ? max : a + b;
}

struct Config {
  bool auto_scale = true;
  int warmup_iters = 2;
  int bench_iters = BENCH_MIN_ITERS;
  uint64_t problem_size = DEFAULT_PROBLEM_SIZE;

  long get_total_iters() { return saturating_add(warmup_iters, bench_iters); }
};
// Global conf object, instantiated in bench.cpp
extern Config conf;

class TimingCollector {
  struct Duration {
    double secs;

    friend std::ostream &operator<<(std::ostream &os, Duration d) {
      if (d.secs < 1e-5)
        return os << d.secs * 1e9 << "ns";
      if (d.secs < 1e-2)
        return os << d.secs * 1e6 << "µs";
      if (d.secs < 10)
        return os << d.secs * 1e3 << "ms";
      return os << d.secs << "s";
    }
  };

  double min_s = 0, max_s = 0, avg_s = 0, total_s = 0;
  size_t *failures;
  uint64_t problem_size;
  std::string test_name;
  std::vector<double> times;

  void compute_result() {
    // Note that this ignores the first <warmup_iters> timings.
    if (times.size() <= conf.warmup_iters)
      return;
    auto times_begin = times.begin() + conf.warmup_iters;
    size_t times_size = times.size() - conf.warmup_iters;

    const auto mm = std::minmax_element(times_begin, times.end());
    min_s = *mm.first;
    max_s = *mm.second;
    avg_s = std::accumulate(times_begin, times.end(), 0.0) /
            static_cast<double>(times_size);
  }

  void print() {
    std::ostream os(std::cout.rdbuf());
    os << bench_name << "|" << test_name << "|n=" << problem_size
       << "|i=" << times.size() - conf.warmup_iters << std::fixed
       << std::setprecision(0) << "|d=" << Duration{total_s} << " - "
       << "min: " << Duration{min_s} << "; max: " << Duration{max_s}
       << "; avg: " << Duration{avg_s};
    if (failures)
      os << (*failures ? " - FAILED" : " - OK");
    os << "\n";
  }

public:
  // num = expected number of iterations
  // data_bytes = bytes handled per iteration
  TimingCollector(std::string_view test_name, size_t *failures = nullptr,
                  std::optional<uint64_t> problem_size = std::nullopt)
      : failures{failures},
        problem_size{problem_size.value_or(conf.problem_size)},
        test_name{test_name} {
    times.reserve(conf.warmup_iters + conf.bench_iters);
  }

  ~TimingCollector() {
    compute_result();
    print();
  }

  void add_duration(double d) {
    times.emplace_back(d);
    if (times.size() > conf.warmup_iters)
      total_s += d;
  }

  bool done() const {
    return conf.auto_scale &&
           times.size() - conf.warmup_iters >= BENCH_MIN_ITERS &&
           total_s >= AUTO_SCALE_TIME;
  }
};

class Timing {
  TimingCollector &tc;
  Clock::time_point start;

public:
  Timing() = delete;
  Timing(TimingCollector &tc) : tc{tc} { start = Clock::now(); }

  ~Timing() { tc.add_duration(duration_cast(Clock::now() - start).count()); }
};
