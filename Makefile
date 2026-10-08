# Copyright © Advanced Micro Devices, Inc., or its affiliates.
#
# SPDX-License-Identifier:  MIT

JOBS := $(shell n=$$(nproc); if [ $$n -le 32 ]; then echo $$n; else echo $$((n / 4)); fi)
MAKEFLAGS += -j$(JOBS) -O

-include local.mk

OFFLOAD_ARCH  ?= gfx90a

CXX_BENCH     ?= clang++
FLAGS_BENCH   ?= -O2 -fopenmp --offload-arch=$(OFFLOAD_ARCH) -std=c++20

CXX_BENCH     := $(patsubst ~/%,$(HOME)/%,$(CXX_BENCH))

# Thanks to BUILD_RULE, BENCHS' elements are also available as make targets.
BENCHS         = omp_launch_latency omp_shared_mem
SRC_DIR        = src
COMMON         = $(SRC_DIR)/bench.cpp
COMMON_HEADERS = $(SRC_DIR)/common.h

.PHONY: all clean format

all: $(BENCHS)

define BUILD_RULE
$B: $(SRC_DIR)/$(B).cpp $(COMMON) $(COMMON_HEADERS)
	@command -v "$(CXX_BENCH)" >/dev/null || { echo "ERROR: CXX '$(CXX_BENCH)' not existing/executable"; exit 1; }
	@echo "Building $(B) ..."
	"$(CXX_BENCH)" $(FLAGS_BENCH) $(SRC_DIR)/$(B).cpp $(COMMON) -o $$@
endef
$(foreach B,$(BENCHS),$(eval $(call BUILD_RULE,$(B))))

format:
	clang-format -i $(SRC_DIR)/*.cpp $(SRC_DIR)/*.h

clean:
	rm -rf $(BENCHS)

