#!/bin/sh
#

# Make sure you have the tools installed:
# brew install graphviz gv google-perftools

export CPUPROFILE=/tmp/bin.prof
make -C tests CFLAGS_EXTRA="-lprofiler -Wl,-no_pie"
pprof --gif ./tests/unit_test_cxx /tmp/bin.prof > /tmp/v7.gif
open /tmp/v7.gif
