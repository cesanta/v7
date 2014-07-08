#PROF = -fprofile-arcs -ftest-coverage -g -O0
CFLAGS = -W -Wall -pedantic -ansi -Wno-comment -g -O0 $(PROF) $(CFLAGS_EXTRA)

all: unit_test
	valgrind -q --leak-check=full ./unit_test
#	gcov -a unit_test.c

unit_test: v7.c v7.h unit_test.c
	g++ v7.c unit_test.c -o $@ $(CFLAGS)

u:
	$(CC) v7.c unit_test.c -o $@ -Weverything -Werror $(CFLAGS)
	./$@

v7: v7.c v7.h
	$(CC) v7.c -o $@ -DV7_EXE $(CFLAGS)

js: v7
	./v7 unit_test.js

w:
	wine cl unit_test.c v7.c && wine unit_test.exe
