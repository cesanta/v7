CFLAGS = -DEJS_UNIT_TEST -W -Wall -g -O0 $(CFLAGS_EXTRA)

all:
	g++ unit_test.c -o ejs $(CFLAGS) && ./ejs
