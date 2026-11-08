# The Art of C++ / PEGTL
# https://github.com/PEGTL

# Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

.SUFFIXES:
.SECONDARY:

ifeq ($(OS),Windows_NT)
UNAME_S := $(OS)
ifeq ($(shell gcc -dumpmachine),mingw32)
MINGW_CXXFLAGS = -U__STRICT_ANSI__
endif
else
UNAME_S := $(shell uname -s)
endif

# For Darwin (Mac OS X / macOS) we assume that the default compiler
# clang++ is used; when $(CXX) is some version of g++, then
# $(CXXSTD) has to be set to -std=c++23 (or newer) so
# that -stdlib=libc++ is not automatically added.

ifeq ($(CXXSTD),)
CXXSTD := -std=c++23
ifeq ($(UNAME_S),Darwin)
CXXSTD += -stdlib=libc++
endif
endif

# Ensure strict standard compliance and no warnings, can be
# changed if desired.

CPPFLAGS ?= -pedantic
CXXFLAGS ?= -Wall -Wextra -Wshadow -Werror -O3 $(MINGW_CXXFLAGS)

LIBRARY_HEADERS := $(shell find include -name '*.hpp')
COMPILE_SOURCES := $(wildcard src/compile/*.cpp)
COMPILE_HEADERS := $(wildcard src/compile/*.hpp)

ifneq ($(MAKECMDGOALS),clean)
FLOATING_CHARCONV := $(shell $(CXX) $(CXXSTD) -Iinclude $(CPPFLAGS) $(CXXFLAGS) -dM -E -x c++ include/tao/pegtl/extra/internal/charconv_features.hpp | grep -c ' TAO_PEGTL_HAS_FLOATING_POINT_FROM_CHARS 1$$')
ifeq ($(FLOATING_CHARCONV),0)
COMPILE_SOURCES := $(filter-out src/compile/extra_charconv_floating%,$(COMPILE_SOURCES))
endif
endif

NORMAL_SOURCES := $(filter-out src/compile/%,$(shell find src -name '*.cpp'))
NORMAL_DEPENDS := $(NORMAL_SOURCES:src/%.cpp=build/dep/%.d)
NORMAL_BINARIES := $(NORMAL_SOURCES:src/%.cpp=build/bin/%)

UNIT_TESTS := $(filter build/bin/test/%,$(NORMAL_BINARIES))
COMPILE_ACCEPT_OBJECTS := $(COMPILE_SOURCES:src/compile/%.cpp=build/compile/accept/%.o)
COMPILE_REJECT_STAMPS := $(COMPILE_SOURCES:src/compile/%.cpp=build/compile/reject/%.stamp)
COMPILE_TESTS := $(COMPILE_ACCEPT_OBJECTS) $(COMPILE_REJECT_STAMPS)

.PHONY: all
all: build check

# Switching between 'icu' and 'all' targets needs a 'make clean',
# and dependencies aren't correct for the ICU test programs yet.

.PHONY: icu
icu: CPPFLAGS += -DTAO_PEGTL_TEST_ICU
icu: CXXFLAGS += -licucore
icu: build check

.PHONY: build
build: $(NORMAL_BINARIES)

.PHONY: tests
tests: run_unit_tests compile_tests

.PHONY: check
check: run_unit_tests compile_tests

.PHONY: run_unit_tests
run_unit_tests: $(UNIT_TESTS)
	@set -e; for T in $(UNIT_TESTS); do echo $$T; $$T > /dev/null; done

.PHONY: compile_tests
compile_tests: $(COMPILE_TESTS)

.PHONY: clean
clean:
	@rm -rf build/*
	@find . -name '*~' -delete

build/dep/%.d: src/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) -Iinclude $(CPPFLAGS) -MM -MQ $@ $< -o $@

build/bin/%: src/%.cpp build/dep/%.d
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) -Iinclude $(CPPFLAGS) $(CXXFLAGS) $< $(LDFLAGS) -o $@

build/compile/accept/%.o: src/compile/%.cpp Makefile $(LIBRARY_HEADERS) $(COMPILE_HEADERS)
	@mkdir -p $(@D)
	@$(CXX) $(CXXSTD) -Iinclude $(CPPFLAGS) $(CXXFLAGS) -DTAO_PEGTL_COMPILE_REJECT=0 -DTAO_PEGTL_COMPILE_ACCEPT=1 -c $< -o $@
	@echo "compile accept" $<

build/compile/reject/%.stamp: src/compile/%.cpp Makefile $(LIBRARY_HEADERS) $(COMPILE_HEADERS)
	@mkdir -p $(@D)
	@if $(CXX) $(CXXSTD) -Iinclude $(CPPFLAGS) $(CXXFLAGS) -DTAO_PEGTL_COMPILE_REJECT=1 -DTAO_PEGTL_COMPILE_ACCEPT=0 -c $< -o $(@:.stamp=.o) > $(@:.stamp=.log) 2>&1; then \
		echo "$< compiled successfully -- should have failed!" && false; \
	else \
		echo "compile reject" $<; touch $@; \
	fi

ifeq ($(findstring $(MAKECMDGOALS),clean),)
-include $(NORMAL_DEPENDS)
endif
