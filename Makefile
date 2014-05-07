#PROF = -fprofile-arcs -ftest-coverage -g -O0
CFLAGS = -W -Wall -g -O0 $(PROF) $(CFLAGS_EXTRA)

all:
	g++ v7.c unit_test.c -o unit_test $(CFLAGS)
	./unit_test
#	gcov -a unit_test.c

w:
	wine cl unit_test.c && wine unit_test.exe
