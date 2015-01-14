# Copyright (c) 2014 Cesanta Software Limited
# All rights reserved
#
# = Requires:
#
# - SRC_DIR = path to directory with source files
# - AMALGAMATED_SOURCES = path to amalgamated C file(s)
# - PROG = name of main unit test source file
#
# - CFLAGS = default compiler flags
# - LDFLAGS = default linker flags
#
# = Parameters:
#
# - V=1 -> show commandlines of executed commands
# - TEST_FILTER -> test name (substring match)
# - CMD -> run wrapped in cmd (e.g. make test_cxx CMD=lldb)
#
# = Useful targets
#
# - compile:   perform a very fast syntax check for all dialects
# - presubmit: suggested presubmit tests
# - cpplint:   run the linter
# - lcov:      generate coverage HTML in test/lcov/index.html
# - test_asan: run with AddressSanitizer
# - test_valgrind: run with valgrind

SRC = $(realpath $(PROG).c)
CLANG:=clang

# OSX clang doesn't build ASAN. Use brew:
#  $ brew tap homebrew/versions
#  $ brew install llvm35 --with-clang --with-asan
ifneq ("$(wildcard /usr/local/bin/clang-3.5)","")
	CLANG:=/usr/local/bin/clang-3.5
endif

PEDANTIC=$(shell gcc --version 2>/dev/null | grep -q clang && echo -pedantic)

###

# TODO(mkm) fix cxx issues. This file should be the same file used in fossa
# and we should keep it in sync with subtree or something.
#DIALECTS=cxx ansi c99 c11
DIALECTS=ansi c99 c11
SPECIALS=asan gcov valgrind

# Each test target might require either a different compiler name
# a compiler flag, or a wrapper to be invoked before executing the test
# they can be overriden here with <VAR>_<target>
#
# Vars are:
# - CC: compiler
# - CFLAGS: flags passed to the compiler, useful to set dialect and to disable incompatible tests
# - LDFLAGS: flags passed to the compiler only when linking (e.g. not in syntax only)
# - SOURCES: non-test source files. To be overriden if needs to build on non amalgamated files
# - CMD: command wrapper or env variables required to run the test binary

CMD=MallocLogFile=/dev/null

CC_cxx=$(CXX)
CFLAGS_cxx=-x c++

CFLAGS_ansi=$(PEDANTIC) -ansi
CFLAGS_c99=$(PEDANTIC) -std=c99

CFLAGS_gcov=$(PEDANTIC) -std=c99 -fprofile-arcs -ftest-coverage
SOURCES_gcov=$(addprefix $(SRC_DIR)/, $(SOURCES))

# TODO(mkm): remove once v7 has #lines for amalgamated sources
SOURCES_asan=$(addprefix $(SRC_DIR)/, $(SOURCES))

CC_asan=$(CLANG)
CFLAGS_asan=-fsanitize=address -fcolor-diagnostics -std=c99
CMD_asan=ASAN_SYMBOLIZER_PATH=/usr/local/bin/llvm-symbolizer-3.5 ASAN_OPTIONS=allocator_may_return_null=1,symbolize=1 $(CMD)

CMD_valgrind=valgrind

###

SHELL := /bin/bash

SUFFIXES=$(DIALECTS) $(SPECIALS)

ALL_PROGS=$(foreach p,$(SUFFIXES),$(PROG)-$(p))
ALL_TESTS=$(foreach p,$(SUFFIXES),test_$(p))
SHORT_TESTS=$(foreach p,$(DIALECTS),test_$(p))

all: clean compile $(SHORT_TESTS)
alltests: $(ALL_TESTS) lcov cpplint

# currently both valgrind and asan tests are failing for some test cases
# it's still useful to be able to run asan/valgrind on some specific test cases
# but we don't enforce them for presubmit until they are stable again.
presubmit: $(SHORT_TESTS) cpplint

.PHONY: clean clean_coverage lcov valgrind docker cpplint
ifneq ($(V), 1)
.SILENT: $(ALL_PROGS) $(ALL_TESTS)
endif

compile:
	@make -j8 $(foreach p,$(DIALECTS),$(PROG)-$(p)) CFLAGS_EXTRA=-fsyntax-only LDFLAGS=

# HACK: cannot have two underscores
$(PROG)-%: Makefile $(SRC) $(or $(SOURCES_$*), $(AMALGAMATED_SOURCES))
	@echo -e "CC\t$(PROG)_$*"
	$(or $(CC_$*), $(CC)) $(CFLAGS_$*) $(SRC) $(or $(SOURCES_$*), $(AMALGAMATED_SOURCES)) -o $(PROG)_$* $(CFLAGS) $(LDFLAGS)

$(ALL_TESTS): test_%: Makefile $(PROG)-%
	@echo -e "RUN\t$(PROG)_$* $(TEST_FILTER)"
	@$(or $(CMD_$*), $(CMD)) ./$(PROG)_$* $(TEST_FILTER)

coverage: Makefile clean_coverage test_gcov
	@echo -e "RUN\tGCOV"
	@gcov -p $(SRC) $(notdir $(SOURCES)) >/dev/null

test_leaks: Makefile
	$(MAKE) test_valgrind CMD_valgrind="$(CMD_valgrind) --leak-check=full"

lcov: clean coverage
	@echo -e "RUN\tlcov"
	@lcov -q -o lcov.info -c -d . 2>/dev/null
	@genhtml -q -o lcov lcov.info

cpplint:
	@echo -e "RUN\tcpplint"
	@cpplint.py --verbose=0 --extensions=c,h $(SRC_DIR)/*.[ch] 2>&1 >/dev/null| grep -v "Done processing" | grep -v "file excluded by"

clean: clean_coverage
	@echo -e "CLEAN\tall"
	@rm -rf $(PROG) $(PROG)_ansi $(PROG)_c99 $(PROG)_gcov $(PROG)_asan $(PROG)_cxx $(PROG)_valgrind lcov.info *.txt *.exe *.obj *.o a.out *.pdb *.opt

clean_coverage:
	@echo -e "CLEAN\tcoverage"
	@rm -rf *.gc* *.dSYM index.html
