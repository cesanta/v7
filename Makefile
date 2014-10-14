WARNS = -Wno-comment -Wno-variadic-macros
V7_FLAGS = -I$(SLRE) -I.
CFLAGS = -W -Wall -pedantic $(WARNS) -g -O0 $(PROF) $(V7_FLAGS) $(CFLAGS_EXTRA)
SOURCES = src/global_vars.c src/util.c src/crypto.c src/array.c src/boolean.c \
          src/date.c src/error.c src/function.c src/math.c src/number.c \
          src/object.c src/regex.c src/rune.c src/runetype.c src/string.c \
		  src/json.c src/stdlib.c src/parser.c src/tokenizer.c src/api.c src/main.c

all: v7 unit_test

v7.c: src/v7_license.h src/utf.h src/internal.h $(SOURCES) v7.h Makefile
	cat src/v7_license.h src/utf.h src/internal.h $(SOURCES) | sed -E '/#include .(v7_license|utf|internal).h./d' > $@

v: unit_test
	valgrind -q --leak-check=full --show-reachable=yes \
	--leak-resolution=high --num-callers=100 ./unit_test
#	valgrind -q --leak-check=full ./v7 tests/run_tests.js
#	gcov -a unit_test.c

unit_test: $(SOURCES) v7.h tests/unit_test.c src/v7_license.h src/utf.h src/internal.h
	g++ $(SOURCES) tests/unit_test.c -o $@ -DV7_PRIVATE="" $(CFLAGS)

xrun: unit_test
	$(CC) -W -Wall -I. -I../slre src/tokenizer.c -DTEST_RUN -DV7_PRIVATE= -o t
	./t

run: unit_test
	./unit_test

all_warnings: v7.c
	$(CC) v7.c tests/unit_test.c -o $@ -Weverything -Werror $(CFLAGS)
	./$@

v7: src/v7_license.h src/utf.h src/internal.h $(SOURCES) v7.h v7.c
	$(CC) $(SOURCES) -o $@ -DV7_EXE -DV7_PRIVATE="" $(CFLAGS) -lm
#	$(CC) $(SOURCES) -o $@ -DV7_EXE $(CFLAGS) -lm

js: v7
	@./v7 tests/v7_basic_test.js
	@rhino -version 130 tests/v7_basic_test.js

t: v7
	./v7 tests/run_ecma262_tests.js

w: v7.c
	wine cl tests/unit_test.c $(SOURCES) $(V7_FLAGS) /Zi -DV7_PRIVATE=""
	wine unit_test.exe

clean:
	rm -rf *.gc* *.dSYM *.exe *.obj *.pdb a.out u unit_test v7 t
