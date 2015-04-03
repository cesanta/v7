CLANG:=clang
CLANG_FORMAT:=clang-format
CLANG_TIDY:=clang-tidy
LLVM_SYMBOLIZER:=/usr/bin/llvm-symbolizer

# Needed by presubmit hook, has to run on OSX as well

# installable with: `brew install llvm36 --with-clang`
ifneq ("$(wildcard /usr/local/bin/clang-format-3.6)","")
	CLANG_FORMAT:=/usr/local/bin/clang-format-3.6
endif

# might be useful to use lldb
ifneq ("$(wildcard /usr/local/bin/clang-3.5)","")
	CLANG:=/usr/local/bin/clang-3.5
endif

#### TODO(mkm): split

# disable optimizations and sockets on windows
DEFS_WINDOWS=-DV7_DISABLE_SOCKETS
CFLAGS_WINDOWS:=-O0 $(DEFS_WINDOWS)
CFLAGS_PLATFORM:=

# not all environments set the same env vars
# so we have to try a few
ifdef SYSTEMROOT
	CFLAGS_PLATFORM:=$(CFLAGS_WINDOWS)
endif
ifeq ($(OS),Windows_NT)
	CFLAGS_PLATFORM:=$(CFLAGS_WINDOWS)
endif

CFLAGS_PLATFORM += $(shell echo -e "\#ifdef __GNUC__\n\#error IS_GCC\n\#endif" | $(CC) -E - 2>&1 | grep -q IS_GCC | echo -Wno-clobbered)
