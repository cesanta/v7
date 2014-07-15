#PROF = -fprofile-arcs -ftest-coverage -g -O0
CFLAGS = -W -Wall -pedantic -ansi -Wno-comment -g -O0 $(PROF) $(CFLAGS_EXTRA)
SLRE = ../slre
CFLAGS += -I$(SLRE)

SOURCES = v7.c $(SLRE)/slre.c

all: unit_test
	valgrind -q --leak-check=full ./unit_test
#	gcov -a unit_test.c

$(SLRE)/slre.c:
	cd .. && git clone https://github.com/cesanta/slre

unit_test: $(SOURCES) v7.h unit_test.c
	g++ $(SOURCES) unit_test.c -o $@ $(CFLAGS)

u:
	$(CC) $(SOURCES) unit_test.c -o $@ -Weverything -Werror $(CFLAGS)
	./$@

v7: $(SOURCES) v7.h
	$(CC) $(SOURCES) -o $@ -DV7_EXE $(CFLAGS)

js: v7
	./v7 unit_test.js

w:
	wine cl unit_test.c $(SOURCES) /I$(SLRE) && wine unit_test.exe
