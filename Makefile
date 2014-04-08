CFLAGS = -W -Wall $(CFLAGS_EXTRA)
#PROF = -fprofile-arcs -ftest-coverage -g -O0

all:
	g++ unit_test.c -o unit_test $(PROF) $(CFLAGS) && ./unit_test
#	gcov -a unit_test.c

w:
	wine cl unit_test.c && wine unit_test.exe
