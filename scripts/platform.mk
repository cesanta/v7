CLANG:=clang
# installable with: `brew install llvm36 --with-clang`
CLANG_FORMAT:=/usr/bin/clang-format-3.6
ifneq ("$(wildcard /usr/local/bin/clang-format-3.6)","")
	CLANG_FORMAT:=/usr/local/bin/clang-format-3.6
endif

ifneq ("$(wildcard /usr/local/bin/clang-3.5)","")
	CLANG:=/usr/local/bin/clang-3.5
endif

LLVM_SYMBOLIZER:=/usr/bin/llvm-symbolizer

ifneq ("$(wildcard /usr/local/bin/llvm-symbolizer-3.5)","")
  LLVM_SYMBOLIZER:=/usr/local/bin/llvm-symbolizer-3.5
endif

# disable optimizations and sockets on windows
CFLAGS_WINDOWS:=-O0 -DV7_DISABLE_SOCKETS
CFLAGS_PLATFORM:=

# not all environments set the same env vars
# so we have to try a few
ifdef SYSTEMROOT
	CFLAGS_PLATFORM:=$(CFLAGS_WINDOWS)
endif
ifeq ($(OS),Windows_NT)
	CFLAGS_PLATFORM:=$(CFLAGS_WINDOWS)
endif
