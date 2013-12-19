CFLAGS = -W -Wall -g -O0 $(CFLAGS_EXTRA)

all:
	g++ unit_test.c -o unit_test $(CFLAGS) && ./unit_test

w:
	wine cl unit_test.c && wine unit_test.exe
